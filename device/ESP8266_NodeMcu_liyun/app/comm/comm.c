/*
 * door.c
 *
 *  Created on: Feb 26, 2019
 *      Author: root
 */

#include "comm.h"

#include "comm_uart.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"

#include "osapi.h"

#include "mqtt/mqtt.h"
#include "http/http.h"
#include "http/http_process.h"
#include "http/http_helper.h"

#include "user_config.h"
#include "osapi.h"

#include "user_interface.h"

#include "json/cJson.h"

#include "sensor/ds18b20.h"
#include "sensor/dht22.h"

#include "comm/comm_pub_def.h"
#include "comm/comm_uart.h"
#include "comm/comm_wifi.h"
#include "comm/comm_pwm.h"
#include "comm/comm_light.h"
#include "comm/comm_sensor.h"
#include "comm/comm_espnow.h"
#include "comm/comm_relay.h"

#include "mqtt/app.h"
#include "http/app.h"

typedef int (*comm_operator_func)(void *);

typedef struct{
	const char *req_type;
	const char *res_type;
	enum MasterMsgType msg_type;
	comm_operator_func func;
	int flag;
}comm_def;

static ErrorMsg error_msg[ErrorCodeSize]={
		{EC_Normal,			""},
		{EC_None,			"None Element"},
		{EC_Failed,			"Request Failed"},
		{EC_Busy,			"Device Busy"},
		{EC_Unknown,		"Unknown Error"}
};
static comm_def comm_operator[]={
		{"SetMotorConfig",			"Reply_SetMotorCw",			NULL,				comm_motor_config_set,			NULL},
		{"GetConnectSynData",	"Reply_GetConnectSynData",	NULL,				comm_connect_syn,			NULL},
		{"GetWiFiConfig",		"Reply_GetWiFiConfig",		NULL,				comm_wifi_config_read,		NULL},
		{"SetWiFiConfig",		"Reply_SetWiFiConfig",		NULL,				comm_wifi_config_write,		NULL},
		{"SetLigthDuty",		"Reply_SetLigthDuty",		NULL,				comm_led_pwm_duty_write,	NULL},
		{"GetRayValue",			"Reply_GetRayValue",		NULL,				comm_ray_value_read,		NULL},
		{"SetRayAlarmValue",	"Reply_SetRayAlarmValue",	NULL,				comm_alarm_ray_value_write,	NULL},
		{"GetWiFiScan",			"Reply_GetWiFiScan",		WIFI_SCAN,			comm_expect_ret,			NEEDTIMER},
		{"WiFiConnect",			"Reply_WiFiConnect",		WIFI_CONNECT,		comm_expect_ret,			NEEDTIMER},
		{"Control",				"Reply_Control",			SYN_CONTROL,		comm_expect_ret,			NEEDTIMER},
		{"GetStatus",			"Reply_GetStatus",			SYN_STATE,			comm_syn_status_read,		NULL},
		{NULL,					NULL,						NULL,				NULL,						NULL},

};

//extern door_comm_buf comm_buf;
uint16 ray_alarm_value=35;
os_timer_t doorRequestTimer;
os_timer_t statusTimer;

typedef int(*door_http_send)(http_connection *);
typedef int(*door_mqtt_publish)(MQTT_Client *);

uint8 temperature_read_tick=0;
static void statusTimerCb(void *arg){
	temperature_read_tick++;
	if(temperature_read_tick==8){
		temperature_read_tick=0;
		/*ds18b20_data ds18b20_read_data;
		if(ds18b20_read(&ds18b20_read_data)==1){
			syn_state.temperature_pre=(int)ds18b20_read_data.temp;
			syn_state.temperature_back=(uint)((ds18b20_read_data.temp-syn_state.temperature_pre)*10);
		}*/
		if( 0 )
			comm_dht22_espnow_read();
		//syn_state.ray=comm_ray_value_read();
		syn_state.temperature=comm_temperature_value_read_api();
		syn_state.humidity=comm_humidity_value_read_api();
	}
	if( 0 )
		comm_ray_value_espnow_read();
	/*if(comm_ray_value_api_get()<ray_alarm_value){
		light_alarm_close();	//set hight
	}else{
		light_alarm_open();		//set low
	}
	*/
	syn_state.run_time++;
}

void ICACHE_FLASH_ATTR comm_init(){
	comm_uart_init();
	os_memset(&doorRequestTimer,0,sizeof(os_timer_t));
	os_timer_disarm(&doorRequestTimer);
	os_timer_setfn(&doorRequestTimer, (os_timer_func_t *)door_request_all_config, NULL);
	os_timer_arm(&doorRequestTimer, 500, 1);

    //ds18b20_init(1);
    comm_espnow_init();
    comm_relay_init();
    comm_sensor_init();
    comm_light_init();
    os_memset(&statusTimer,0,sizeof(os_timer_t));
    os_timer_disarm(&statusTimer);
    os_timer_setfn(&statusTimer, (os_timer_func_t *)statusTimerCb, NULL);
    os_timer_arm(&statusTimer, 1000, 1);
}

int ICACHE_FLASH_ATTR http_operator_api(http_connection *c){
	//wait for whole body
	if(c->state < HTTPD_STATE_BODY_END)
		return HTTPD_CGI_MORE;
	c->sign=CLIENT_IS_HTTP;
	return common_operator_api(c);
}


int ICACHE_FLASH_ATTR mqtt_operator_api(const char* data,uint32 len,const char *ptopic,uint8 pqos){
	mqtt_user_data *user_data;
	//must copy
	user_data=(mqtt_user_data *)os_zalloc(sizeof(mqtt_user_data));
	user_data->data = (char*)os_zalloc(len+1);
	user_data->sign=CLIENT_IS_MQTT;
	os_memset(user_data->data,0,len+1);
	os_memcpy(user_data->data, data, len);
	user_data->ptopic=ptopic;
	user_data->pqos=pqos;
	return common_operator_api((void *)user_data);
}

const char *ICACHE_FLASH_ATTR get_res_type(void *client){
	uint8_t c_sign=*((uint8_t *)client);
	char *data=(c_sign==CLIENT_IS_HTTP?(((http_connection *)client)->body.data):(((mqtt_user_data *)client)->data));
	cJSON *root=cJSON_Parse(data);
	char *type=cJSON_GetObjectItem(root,"type")->valuestring;
	int index;
	for(index=0;comm_operator[index].req_type!=NULL;index++){
		if(strcmp(type,comm_operator[index].req_type)==0){
			cJSON_Delete(root);
			return comm_operator[index].res_type;
		}
	}
	cJSON_Delete(root);
	return "";
}

uint8 ICACHE_FLASH_ATTR send_ret_json(void *client,cJSON *retroot,enum ErrorCode error_code){
	cJSON_AddStringToObject(retroot, "type",get_res_type(client));
	cJSON_AddNumberToObject(retroot, "error_code",error_code );
	int i;
	for(i=0;i<ErrorCodeSize;i++){
		if(error_msg[i].error_code==error_code){
			cJSON_AddStringToObject(retroot, "error_str",error_msg[i].error_str);
			break;
		}
	}
	char *ostream=cJSON_Print(retroot);
	u8 ret=send_to_client(client,ostream);
	os_free(ostream);
	cJSON_Delete(retroot);
	return ret;
}

//mqtt_publish_api(const char* topic, const char* data, int data_length, int qos, int retain);


int ICACHE_FLASH_ATTR common_operator_api(void *client){
	char *data;
	mqtt_user_data *mqtt_data;
	uint8_t client_sign=*(uint8_t *)client;
	http_connection *http_client;
	if(client_sign==CLIENT_IS_MQTT){
		mqtt_data=(mqtt_user_data *)client;
		data = mqtt_data->data;
	}else if(client_sign==CLIENT_IS_HTTP){
		http_client=(http_connection *)client;
		data=http_client->body.data;
	}

	//parse json
	cJSON *root = cJSON_Parse(data);
	if(root==NULL){
		goto badJson;
	}
	cJSON *type = cJSON_GetObjectItem(root,"type");
	if(type==NULL){
		goto badJson;
	}
	NODE_DBG("request type :%s",type->valuestring);

	int index,ret;
	int type_is_exist=0;
	for(index=0;comm_operator[index].req_type!=NULL;index++){
		if(strcmp(type->valuestring,comm_operator[index].req_type)==0){
			type_is_exist=1;if(comm_operator[index].flag==NULL){
				if(client_sign==CLIENT_IS_HTTP){
					//set header
					http_SET_HEADER(client,HTTP_CONTENT_TYPE,JSON_CONTENT_TYPE);
					http_response_OK(client);
				}
			}else if(comm_operator[index].flag==NEEDTIMER){
				client_handle_timer *Timer;
				if(client_sign==CLIENT_IS_MQTT){
					Timer=&mqtt_data->Timer;
				}else if(client_sign==CLIENT_IS_HTTP){
					Timer=(client_handle_timer *)os_zalloc(sizeof(client_handle_timer));
					http_client->cgi.data=Timer;
					http_client->cgi.function=(http_callback)comm_operator[index].func;
				}
				Timer->msgtype=comm_operator[index].msg_type;


				//send to Master
				switch(comm_operator[index].msg_type){
					case NORMAL_CONFIG:
						door_config_refresh_set(COMM_NOREFRESH);
						send_message_to_master(comm_operator[index].msg_type,data,os_strlen(data));
						break;
					case SYN_CONTROL:
						do{
							cJSON *c_data=cJSON_GetObjectItem(root,"data");
							uint8 d_type=cJSON_GetObjectItem(c_data,"device_type")->valueint;
							uint8 index=cJSON_GetObjectItem(c_data,"index")->valueint;
							uint8 op=cJSON_GetObjectItem(c_data,"op")->valueint;
							switch(d_type){
							  case DT_Relay:
								comm_relay_refresh_set(index,COMM_NOREFRESH);
								comm_relay_status_set_app_api(index,op);
							}
							//syn_control_refresh_set(COMM_REFRESHED);
							//send_message_to_master(SYN_CONTROL,&typevaule,1);
						}while(0);
						break;
					case WIFI_SCAN:
						NODE_DBG("msg type is:NORMAL_WIFI_SCAN");

						break;
				}
				Timer->tickcount=0;
			}
			cJSON_Delete(root);
			return comm_operator[index].func(client);
		}
	}
	cJSON_Delete(root);
	if(type_is_exist==0){
		os_printf("didn't has this type.\r\n");
		return send_to_client(client,"error");
	}

badJson:
os_printf("badJson.\r\n");
	if(root!=NULL){
		cJSON_Delete(root);
	}
	if(client_sign==CLIENT_IS_HTTP){
		http_response_BAD_REQUEST(client);
		return HTTPD_CGI_DONE;
	}
	if(client_sign==CLIENT_IS_MQTT){
		//send_to_client(client,"error");
		if(mqtt_data->data!=NULL)os_free(mqtt_data->data);
		if(mqtt_data!=NULL)os_free(mqtt_data);
	}
	return 1;
}



int ICACHE_FLASH_ATTR send_to_client(void *client,char *message){
	NODE_DBG("send_to_client....");
	if(*(uint8_t *)client==CLIENT_IS_MQTT){
		mqtt_user_data *mqtt_data=(mqtt_user_data *)client;
		NODE_DBG("send_to_client public.");
		mqtt_publish_api(mqtt_data->ptopic,message,os_strlen(message),mqtt_data->pqos,0);
		if(mqtt_data->data!=NULL)
			os_free(mqtt_data->data);
		if(mqtt_data!=NULL)
			os_free(mqtt_data);
	}else if(*(uint8_t *)client==CLIENT_IS_HTTP){
		http_write(client,message);
		if(((http_connection *)client)->cgi.data!=NULL)
			os_free(((http_connection *)client)->cgi.data);
		return HTTPD_CGI_DONE;
	}
	return 1;
}

int ICACHE_FLASH_ATTR comm_connect_syn(void *client){
	cJSON *retroot=cJSON_CreateObject();
	cJSON *data,*d_alarm,*d_relay,*relay_array,*device_status,*item;
	cJSON_AddItemToObject(retroot, "data",data=cJSON_CreateObject());

	cJSON_AddItemToObject(data, "device-status",device_status=cJSON_CreateObject());
	cJSON_AddNumberToObject(device_status, "test-1",0);
	cJSON_AddNumberToObject(device_status, "test-1",1);

	cJSON_AddItemToObject(data, "alarm",d_alarm=cJSON_CreateObject());
	cJSON_AddNumberToObject(d_alarm,"ray-value",ray_alarm_value);
	int i;
	cJSON_AddItemToObject(data, "relays",relay_array = cJSON_CreateArray());
	for(i=0;i<RELAYSIZE;i++){
		cJSON_AddItemToArray(relay_array,d_relay = cJSON_CreateObject());
		cJSON_AddNumberToObject(d_relay,"device_type",0);
		cJSON_AddNumberToObject(d_relay,"index",i);
		cJSON_AddNumberToObject(d_relay,"op",comm_relay_status_get_api(i));
	}
	cJSON_AddNumberToObject(data, "duty",comm_led_pwm_duty_api_get());
	return send_ret_json(client,retroot,EC_Normal);
}

int ICACHE_FLASH_ATTR comm_syn_status_read(void *client){
	int ret=HTTPD_CGI_DONE;
	cJSON *retroot,*data;
	retroot=cJSON_CreateObject();
	cJSON_AddItemToObject(retroot,"data",data=cJSON_CreateObject());
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.sm_state),syn_state.sm_state);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.comm_state),syn_state.comm_state);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.temperature),syn_state.temperature);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.humidity),syn_state.humidity);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.power),syn_state.power);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.run_time),syn_state.run_time);
	cJSON_AddNumberToObject(data,"ray-value",comm_ray_value_api_get());
	return send_ret_json(client,retroot,EC_Normal);
}



int ICACHE_FLASH_ATTR comm_wifi_config_read(void *client){
	cJSON *retroot=(cJSON *)comm_wifi_config_read_api();
	return send_ret_json(client,retroot,EC_Normal);
}

int ICACHE_FLASH_ATTR comm_wifi_config_write(void *client){
	uint8 client_sign=*(int *)client;
	cJSON *retroot=cJSON_CreateObject();
	enum ErrorCode error_code=EC_Normal;
	cJSON *root = cJSON_Parse(client_sign==CLIENT_IS_HTTP?(((http_connection *)client)->body.data):(((mqtt_user_data *)client)->data));
	cJSON *root_data = cJSON_GetObjectItem(root,"data");
	if(root_data==NULL){
		error_code=EC_None;
	}else{
		comm_wifi_config_write_api(root_data);
	}
	cJSON_Delete(root);
	return send_ret_json(client,retroot,error_code);
}


int ICACHE_FLASH_ATTR comm_motor_config_set(void *client){
	uint8 client_sign=*(uint8_t *)client;
		enum ErrorCode error_code=EC_Normal;
		cJSON *retroot=cJSON_CreateObject();
		cJSON *ret_data;
		cJSON *root = cJSON_Parse(client_sign==CLIENT_IS_HTTP?(((http_connection *)client)->body.data):(((mqtt_user_data *)client)->data));
		cJSON *root_data = cJSON_GetObjectItem(root,"data");
		if(root_data==NULL){
			error_code=EC_None;
			goto badJson;
		}
		cJSON *data_direction=cJSON_GetObjectItem(root_data,"direction");
		cJSON *data_speed=cJSON_GetObjectItem(root_data,"speed");
		if(!data_direction || !data_speed){
			error_code=EC_None;
			goto badJson;
		}
		motor_direction_set(data_direction->valueint);
		motor_speed_set(data_speed->valueint);
		cJSON_AddItemToObject(retroot,"data",ret_data=cJSON_CreateObject());
		cJSON_AddNumberToObject(ret_data,"direction",data_direction->valueint);
		cJSON_AddNumberToObject(ret_data,"speed",data_speed->valueint);
		//must limits
	badJson:
		if(root!=NULL){
			cJSON_Delete(root);
		}
		return send_ret_json(client,retroot,error_code);

}
int ICACHE_FLASH_ATTR comm_led_pwm_duty_write(void *client){
	uint8 client_sign=*(uint8_t *)client;
	enum ErrorCode error_code=EC_Normal;
	cJSON *retroot=cJSON_CreateObject();
	cJSON *ret_data;
	cJSON *root = cJSON_Parse(client_sign==CLIENT_IS_HTTP?(((http_connection *)client)->body.data):(((mqtt_user_data *)client)->data));
	cJSON *root_data = cJSON_GetObjectItem(root,"data");
	if(root_data==NULL){
		error_code=EC_None;
		goto badJson;
	}
	cJSON *data_duty=cJSON_GetObjectItem(root_data,"duty");
	if(data_duty==NULL){
		error_code=EC_None;
		goto badJson;
	}
	comm_led_pwm_duty_api_set(data_duty->valueint);
	cJSON_AddItemToObject(retroot,"data",ret_data=cJSON_CreateObject());
	cJSON_AddNumberToObject(ret_data,"duty",comm_led_pwm_duty_api_get());
	//must limits
badJson:
	if(root!=NULL){
		cJSON_Delete(root);
	}
	return send_ret_json(client,retroot,error_code);
}


int ICACHE_FLASH_ATTR comm_ray_value_read(void *client){
	//parse json
	cJSON *retroot=cJSON_CreateObject();
	cJSON *data;
	cJSON_AddItemToObject(retroot, "data",data=cJSON_CreateObject() );
	cJSON_AddNumberToObject(data, "ray-value",comm_ray_value_api_get() );
	return send_ret_json(client,retroot,EC_Normal);

}
int ICACHE_FLASH_ATTR comm_alarm_ray_value_write(void *client){
	char *data;
	uint8 client_sign=*(uint8_t *)client;
	http_connection *http_client;
	if(client_sign==CLIENT_IS_MQTT){
		mqtt_user_data *user_data=(mqtt_user_data *)client;
		data = user_data->data;
	}else if(client_sign==CLIENT_IS_HTTP){
		http_client=(http_connection *)client;
		data=http_client->body.data;
	}
	enum ErrorCode error_code=EC_Normal;
	cJSON *retroot=cJSON_CreateObject();
	cJSON *root = cJSON_Parse(data);
	cJSON *c_data;
	c_data = cJSON_GetObjectItem(root,"data");
	if(c_data==NULL){
		error_code=EC_None;
		goto badJson;
	}
	cJSON *data_rav=cJSON_GetObjectItem(c_data,"ray-value");
	if(data_rav==NULL){
		error_code=EC_None;
		goto badJson;
	}
	ray_alarm_value=data_rav->valueint;
	//cJSON_Delete(root);
	cJSON *retdata;
	cJSON_AddItemToObject(retroot,"data", retdata = cJSON_CreateObject());
	cJSON_AddNumberToObject(retdata,"ray-value", ray_alarm_value);

badJson:
	if(root!=NULL){
		cJSON_Delete(root);
	}
	return send_ret_json(client,retroot,error_code);

}

int ICACHE_FLASH_ATTR comm_expect_ret(void *client){
	client_handle_timer *Timer;
	uint8 client_sign=*(uint8_t *)client;
	if(client_sign==CLIENT_IS_HTTP){
		Timer=(client_handle_timer *)((http_connection *)client)->cgi.data;
		if(Timer->tickcount==0){
			os_memset(&Timer->timer,0,sizeof(os_timer_t));
			os_timer_disarm(&Timer->timer);
			os_timer_setfn(&Timer->timer, http_execute_cgi, client);
			os_timer_arm(&Timer->timer, COMM_TIMER_SINGLE_TIME, 1);
			//set header
			http_SET_HEADER(client,HTTP_CONTENT_TYPE,JSON_CONTENT_TYPE);
			http_response_OK(client);
		}
	}else if(client_sign==CLIENT_IS_MQTT){
		Timer=&((mqtt_user_data *)client)->Timer;
		if(Timer->tickcount==0){
			os_memset(&Timer->timer,0,sizeof(os_timer_t));
			os_timer_disarm(&Timer->timer);
			os_timer_setfn(&Timer->timer, comm_expect_ret, client);
			os_timer_arm(&Timer->timer, COMM_TIMER_SINGLE_TIME, 1);
		}
	}
	Timer->tickcount += COMM_TIMER_SINGLE_TIME;
	//NODE_DBG("Timer->tickcount is %d",Timer->tickcount);
	//recv refresh data or timeout
	switch(Timer->msgtype){
	case NORMAL_CONFIG:
		/*
		if(door_config_refresh_get()==COMM_REFRESHED || Timer->tickcount>TIMER_TIMEROUT){
			os_timer_disarm(&Timer->timer);
			//modify config, must request config
			door_config_refresh_set(COMM_NOREFRESH);
			return send_to_client(client,door_config_read());
		}
		break;
		*/
	case SYN_CONTROL:
		{
			cJSON *root=cJSON_Parse(client_sign==CLIENT_IS_HTTP?(((http_connection *)client)->body.data):(((mqtt_user_data *)client)->data));
			cJSON *r_data=cJSON_GetObjectItem(root,"data");
			uint8 index=cJSON_GetObjectItem(r_data,"index")->valueint;
			cJSON *retroot=cJSON_CreateObject();
			if(comm_relay_refresh_status_get(index)==COMM_REFRESHED || Timer->tickcount>COMM_TIMER_TIMEOUT){
				os_timer_disarm(&Timer->timer);
				NODE_DBG("SYN_CONTROL Ret: %s",(comm_relay_refresh_status_get(index)==COMM_REFRESHED?"Refreshed":"Timer timeout"));
				enum ErrorCode error_code = EC_Normal;
				if(comm_relay_refresh_status_get(index)==COMM_REFRESHED){
					uint8 d_type=cJSON_GetObjectItem(r_data,"device_type")->valueint;
					uint8 op=cJSON_GetObjectItem(r_data,"op")->valueint;
					cJSON *ret_data;
					cJSON_AddItemToObject(retroot,"data",ret_data = cJSON_CreateObject());
					cJSON_AddNumberToObject(ret_data,"device_type",d_type);
					cJSON_AddNumberToObject(ret_data,"index",index);
					cJSON_AddNumberToObject(ret_data,"op",op);
				}else{
					error_code=EC_Failed;
				}
				cJSON_Delete(root);
				return send_ret_json(client,retroot,error_code);
			}
			cJSON_Delete(root);
		}
		break;

	case WIFI_CONNECT:
		if(Timer->tickcount==COMM_TIMER_SINGLE_TIME){
			cJSON *root = cJSON_Parse(client_sign==CLIENT_IS_HTTP?(((http_connection *)client)->body.data):(((mqtt_user_data *)client)->data));
			cJSON *root_data = cJSON_GetObjectItem(root,"data");
			char *ssid = cJSON_GetObjectItem(root_data,"wifi_station_ssid")->valuestring;
			char *pwd = cJSON_GetObjectItem(root_data,"wifi_station_pwd")->valuestring;
			if(!comm_wifi_start_connect_ap_api(ssid,pwd)){
				os_timer_disarm(&Timer->timer);
				send_ret_json(client,cJSON_CreateObject(),EC_Busy);
			}
			cJSON_Delete(root);
		}else{
			if(Timer->tickcount>COMM_TIMER_TIMEOUT && comm_wifi_connect_ap_check_api()){
				os_timer_disarm(&Timer->timer);
				cJSON *retroot=cJSON_CreateObject();
				return send_ret_json(client,retroot,EC_Normal);
			}
			if(Timer->tickcount>COMM_TIMER_TIMEOUT * 5){
				os_timer_disarm(&Timer->timer);
				//connect failed, connect default ap
				comm_wifi_connect_default_ap_api();
				cJSON *retroot=cJSON_CreateObject();
				return send_ret_json(client,retroot,EC_Failed);
			}
		}
		break;
	case WIFI_SCAN:
		if(Timer->tickcount==COMM_TIMER_SINGLE_TIME){
			if(!comm_wifi_scan_start_api()){
				os_timer_disarm(&Timer->timer);
				return send_ret_json(client,cJSON_CreateObject(),EC_Busy);
			}
		}
		else{
			cJSON *retroot=(cJSON *)comm_wifi_scan_api();
			if(retroot!=NULL){
				os_timer_disarm(&Timer->timer);
				return send_ret_json(client,retroot,EC_Normal);
			}
			else if(Timer->tickcount>COMM_TIMER_TIMEOUT * 3){
				os_timer_disarm(&Timer->timer);
				retroot=cJSON_CreateObject();
				return send_ret_json(client,retroot,EC_Failed);
			}
		}
		break;
	}
	if(client_sign==CLIENT_IS_HTTP){
		return HTTPD_CGI_MORE;
	}
	return 1;
}

void ICACHE_FLASH_ATTR request_master_door_config(){

	send_message_to_master(NORMAL_CONFIG,"",0);
}
void ICACHE_FLASH_ATTR door_request_all_config(){
	/*
	if(door_config_refresh_get()==COMM_NOREFRESH)
		door_config_refresh_set(COMM_REFRESHING);
	else  if(door_config_refresh_get()==COMM_REFRESHING)
		request_master_door_config();
		*/

}


char *ICACHE_FLASH_ATTR get_struct_last_str(char *str){
	int i = 0;
	for (i = strlen(str)-1; i > 0; i--)
		if (*(str + i) == '.')
		{
			return str + i + 1;
			break;
		}
	return NULL;

}


int ICACHE_FLASH_ATTR door_door_config_read(void *client){
	NODE_DBG("http_door_config_api_read");
	if(door_config_refresh_get()==COMM_REFRESHED)
	{
		return send_to_client(client,door_config_read());
	}
	return send_to_client(client,"error");
}




