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

#include "comm/comm_uart.h"
#include "comm/comm_wifi.h"
#include "comm/comm_pwm.h"
#include "comm/comm_light.h"
#include "comm/comm_sensor.h"
#include "comm/comm_espnow.h"

#include "mqtt/app.h"
#include "http/app.h"

typedef int (*comm_operator_func)(void *);

typedef struct{
	const char *type;
	enum MasterMsgType msg_type;
	comm_operator_func func;
	int flag;
}comm_def;

static ErrorMsg error_msg[ErrorCodeSize]={
		{EC_Normal,			""},
		{EC_None,			"None Element"},
		{EC_Unknown,		"Unknown Error"}
};
static comm_def comm_operator[]={
		{"GetConnectSynData",	NULL,				comm_connect_syn,			NULL},
		{"GetWiFiConfig",		NULL,				comm_wifi_config_read,		NULL},
		{"SetWiFiConfig",		NULL,				comm_wifi_config_write,		NULL},
		{"GetSafeConfig",		NULL,				comm_wifi_safe_read,		NULL},
		{"GetLigthDuty",		NULL,				comm_led_pwm_duty_read,		NULL},
		{"SetLigthDuty",		NULL,				comm_led_pwm_duty_write,	NULL},
		{"GetRayValue",			NULL,				comm_ray_value_read,		NULL},
		{"SetAlarmRayValue",	NULL,				comm_alarm_ray_value_write,	NULL},
		{"GetWiFiScan",			NORMAL_WIFI_SCAN,	comm_expect_ret,			NEEDTIMER},
		{"Control",				SYN_CONTROL,		comm_expect_ret,			NEEDTIMER},
		{"GetStatus",			SYN_STATE,			comm_syn_status_read,		NULL},
		{NULL,					NULL,				NULL,						NULL},

};
//extern door_comm_buf comm_buf;
uint16 ray_alarm_value=9999;
os_timer_t doorRequestTimer;
os_timer_t statusTimer;

typedef int(*door_http_send)(http_connection *);
typedef int(*door_mqtt_publish)(MQTT_Client *);

uint8 temperature_read_tick=0;
static void statusTimerCb(void *arg){
	temperature_read_tick++;
	if(temperature_read_tick==3){
		temperature_read_tick=0;
		ds18b20_data ds18b20_read_data;
		if(ds18b20_read(&ds18b20_read_data)==1){
			syn_state.temperature_pre=(int)ds18b20_read_data.temp;
			syn_state.temperature_back=(uint)((ds18b20_read_data.temp-syn_state.temperature_pre)*10);
		}
		comm_sensor_ray_value_read();
	}
	if(comm_ray_value_api_get()<ray_alarm_value){
		light_alarm_close();
	}else{
		light_alarm_open();
	}
	syn_state.run_time++;
}

void ICACHE_FLASH_ATTR comm_init(){
	comm_uart_init();
	os_memset(&doorRequestTimer,0,sizeof(os_timer_t));
	os_timer_disarm(&doorRequestTimer);
	os_timer_setfn(&doorRequestTimer, (os_timer_func_t *)door_request_all_config, NULL);
	os_timer_arm(&doorRequestTimer, 500, 1);

    ds18b20_init(1);
    comm_pwm_init();
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

int ICACHE_FLASH_ATTR mqtt_operator_api(MQTT_Client *c){
	return common_operator_api(c);
}

cJSON *ICACHE_FLASH_ATTR create_ret_json(char *ret_type){
	cJSON *retroot=cJSON_CreateObject();
	cJSON_AddStringToObject(retroot,"type",ret_type);
	return retroot;
}
uint8 ICACHE_FLASH_ATTR send_ret_json(void *client,cJSON *retroot,enum ErrorCode error_code){
	cJSON_AddNumberToObject(retroot, "error_code",error_code );
	int i;
	if(error_code==0)
		cJSON_AddStringToObject(retroot, "error_str","" );
	else{
		for(i=0;i<ErrorCodeSize;i++){
			if(error_msg[i].error_code==error_code){
				cJSON_AddStringToObject(retroot, "error_str",error_msg[i].error_str);
			}

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
	MqttUserData *mqtt_user_data;
	uint8_t client_sign=*(uint8_t *)client;
	http_connection *http_client;
	mqtt_comm_data *udata;
	if(client_sign==CLIENT_IS_MQTT){
		MQTT_Client *mqtt_client=(MQTT_Client *)client;
		udata=(mqtt_comm_data *)mqtt_client->user_data;
		//must copy
		mqtt_user_data=(MqttUserData *)os_zalloc(sizeof(MqttUserData));
		mqtt_user_data->data = (char*)os_zalloc(udata->data_len+1);

		mqtt_user_data->sign=CLIENT_IS_MQTT;
		os_memcpy(mqtt_user_data->data, udata->msg, udata->data_len);
		mqtt_user_data->data[udata->data_len] = 0;
		data=mqtt_user_data->data;
		mqtt_user_data->ptopic=udata->pub_topic;
		mqtt_user_data->pqos=udata->pub_qos;
		client=(void *)mqtt_user_data;
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
	for(index=0;comm_operator[index].type!=NULL;index++){
		if(strcmp(type->valuestring,comm_operator[index].type)==0){
			cJSON_Delete(root);
			type_is_exist=1;
			if(comm_operator[index].flag==NULL){
				if(client_sign==CLIENT_IS_HTTP){
					//set header
					http_SET_HEADER(client,HTTP_CONTENT_TYPE,JSON_CONTENT_TYPE);
					http_response_OK(client);
				}
			}else if(comm_operator[index].flag==NEEDTIMER){
				client_handle_timer *Timer;
				if(client_sign==CLIENT_IS_MQTT){
					Timer=&mqtt_user_data->Timer;
					NODE_DBG("NEEDTIMER timer is:%d",mqtt_user_data->Timer);
				}else if(client_sign==CLIENT_IS_HTTP){
					Timer=(client_handle_timer *)os_malloc(sizeof(client_handle_timer));
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
							cJSON *c_root=cJSON_Parse(data);
							cJSON *c_data=cJSON_GetObjectItem(c_root,"data");
							uint8 c_type=cJSON_GetObjectItem(c_data,"control_type")->valueint;
							uint8 typevaule=0;
							switch(c_type){
							case INPUT_FLAG_OPEN:
								typevaule=0x08;
								break;
							case INPUT_FLAG_CLOSE:
								typevaule=0x10;
								break;
							case INPUT_FLAG_FREEZE:
								typevaule=0x20;
								break;
							case INPUT_FLAG_UNFREEZE:
								typevaule=0x40;
								break;
							}

							//syn_control_refresh_set(COMM_NOREFRESH);
							//send_message_to_master(SYN_CONTROL,&typevaule,1);
						}while(0);
						break;
					case NORMAL_WIFI_SCAN:
						NODE_DBG("msg type is:NORMAL_WIFI_SCAN");
						break;
				}
				Timer->tickcount=0;
			}
			return comm_operator[index].func(client);
		}
	}
	cJSON_Delete(root);
	if(type_is_exist==0){
		return send_to_client(client,"error");
	}

badJson:
	if(root!=NULL){
		cJSON_Delete(root);
	}
	if(client_sign==CLIENT_IS_HTTP){
		http_response_BAD_REQUEST(client);
		return HTTPD_CGI_DONE;
	}
	if(client_sign==CLIENT_IS_MQTT){
		//send_to_client(client,"error");
		if(mqtt_user_data->data!=NULL)os_free(mqtt_user_data->data);
		if(mqtt_user_data!=NULL)os_free(mqtt_user_data);
	}
	return 1;
}

int ICACHE_FLASH_ATTR send_to_client(void *client,char *message){
	NODE_DBG("send_to_client....");
	if(*(uint8_t *)client==CLIENT_IS_MQTT){
		MqttUserData *mqtt_user_data=(MqttUserData *)client;
		NODE_DBG("send_to_client public.");
		mqtt_publish_api(mqtt_user_data->ptopic,message,os_strlen(message),mqtt_user_data->pqos,0);
		if(mqtt_user_data->data!=NULL)
			os_free(mqtt_user_data->data);
		if(mqtt_user_data!=NULL)
			os_free(mqtt_user_data);
	}else if(*(uint8_t *)client==CLIENT_IS_HTTP){
		http_write(client,message);
		if(((http_connection *)client)->cgi.data!=NULL)
			os_free(((http_connection *)client)->cgi.data);
		return HTTPD_CGI_DONE;
	}
	return 1;
}

int ICACHE_FLASH_ATTR comm_connect_syn(void *client){
	cJSON *retroot=create_ret_json("Reply_ConnectSynData");
	cJSON *data,*d_alarm,*device_status,*item;
	cJSON_AddItemToObject(retroot, "data",data=cJSON_CreateObject());



	cJSON_AddItemToObject(data, "device-status",device_status=cJSON_CreateObject());
	cJSON_AddNumberToObject(device_status, "test-1",0);
	cJSON_AddNumberToObject(device_status, "test-1",1);

	cJSON_AddItemToObject(data, "alarm",d_alarm=cJSON_CreateObject());
	cJSON_AddNumberToObject(d_alarm,"ray-value",ray_alarm_value);

	cJSON_AddNumberToObject(data, "duty",comm_led_pwm_duty_api_get());

	return send_ret_json(client,retroot,EC_Normal);
}

int ICACHE_FLASH_ATTR comm_syn_status_read(void *client){
	int ret=HTTPD_CGI_DONE;
	cJSON *retroot,*data;
	retroot=create_ret_json("Reply_GetStatus");
	cJSON_AddItemToObject(retroot,"data",data=cJSON_CreateObject());
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.sm_state),syn_state.sm_state);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.comm_state),syn_state.comm_state);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.temperature_pre),syn_state.temperature_pre);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.temperature_back),syn_state.temperature_back);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.wetness),syn_state.wetness);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.power),syn_state.power);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.run_time),syn_state.run_time);
	send_ret_json(client,retroot,EC_Normal);
}



int ICACHE_FLASH_ATTR comm_led_pwm_duty_read(void *client){
	//parse json
	cJSON *retroot=create_ret_json("Reply_GetLigthDuty");
	cJSON *data;
	cJSON_AddItemToObject(retroot, "data",data=cJSON_CreateObject() );
	cJSON_AddNumberToObject(data, "duty",comm_led_pwm_duty_api_get() );
	return send_ret_json(client,retroot,EC_Normal);
}



int ICACHE_FLASH_ATTR comm_led_pwm_duty_write(void *client){
	char *data;
	uint8 client_sign=*(uint8_t *)client;
	http_connection *http_client;
	MqttUserData *mqtt_user_data;
	if(client_sign==CLIENT_IS_MQTT){
		mqtt_user_data=(MqttUserData *)client;
		data = mqtt_user_data->data;
	}else if(client_sign==CLIENT_IS_HTTP){
		http_client=(http_connection *)client;
		data=http_client->body.data;
	}
	enum ErrorCode error_code=EC_Normal;
	cJSON *retroot=create_ret_json("Reply_SetLigthDuty");
	cJSON *ret_data;
	cJSON *root = cJSON_Parse(data);
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
	cJSON *retroot=create_ret_json("Reply_GetRayValue");
	cJSON *data;
	cJSON_AddItemToObject(retroot, "data",data=cJSON_CreateObject() );
	cJSON_AddNumberToObject(data, "ray-value",comm_ray_value_api_get() );
	return send_ret_json(client,retroot,EC_Normal);

}
int ICACHE_FLASH_ATTR comm_alarm_ray_value_write(void *client){
	char *data;
		uint8 client_sign=*(uint8_t *)client;
		http_connection *http_client;
		mqtt_comm_data *mqtt_user_data;
		if(client_sign==CLIENT_IS_MQTT){
			MQTT_Client *mqtt_client=(MQTT_Client *)client;
			mqtt_user_data=(mqtt_comm_data *)mqtt_client->user_data;
			data = (char*)os_zalloc(mqtt_user_data->data_len+1);
			os_memcpy(data, mqtt_user_data->msg, mqtt_user_data->data_len);
			data[mqtt_user_data->data_len] = 0;
		}else if(client_sign==CLIENT_IS_HTTP){
			http_client=(http_connection *)client;
			data=http_client->body.data;
		}
		enum ErrorCode error_code=EC_Normal;
		cJSON *retroot=create_ret_json("Reply_SetAlarmRayValue");
		cJSON *root = cJSON_Parse(data);
		cJSON *root_data = cJSON_GetObjectItem(root,"data");
		if(root_data==NULL){
			error_code=EC_None;
			goto badJson;
		}
		cJSON *data_rav=cJSON_GetObjectItem(root_data,"alarm-ray-value");
		if(data_rav==NULL){
			error_code=EC_None;
			goto badJson;
		}
		ray_alarm_value=data_rav->valueint;
	badJson:
		if(root!=NULL){
			cJSON_Delete(root);
		}
		if(client_sign==CLIENT_IS_MQTT){
			os_free(data);
		}
		return send_ret_json(client,retroot,error_code);

}




int ICACHE_FLASH_ATTR comm_expect_ret(void *client){
	NODE_DBG("comm_expect_ret");
	client_handle_timer *Timer;
	uint8 c_sign=*(uint8_t *)client;
	if(c_sign==CLIENT_IS_HTTP){
		Timer=(client_handle_timer *)((http_connection *)client)->cgi.data;
		if(Timer->tickcount==0){
			os_memset(&Timer->timer,0,sizeof(os_timer_t));
			os_timer_disarm(&Timer->timer);
			os_timer_setfn(&Timer->timer, http_execute_cgi, client);
			os_timer_arm(&Timer->timer, TIMER_SINGLETIME, 1);
			//set header
			http_SET_HEADER(client,HTTP_CONTENT_TYPE,JSON_CONTENT_TYPE);
			http_response_OK(client);
		}
	}else if(c_sign==CLIENT_IS_MQTT){

		NODE_DBG("CLIENT_IS_MQTT");
		NODE_DBG(" timer is:%d",&((MqttUserData *)client)->Timer);
		Timer=&((MqttUserData *)client)->Timer;
		if(Timer->tickcount==0){
			os_memset(&Timer->timer,0,sizeof(os_timer_t));
			os_timer_disarm(&Timer->timer);
			os_timer_setfn(&Timer->timer, comm_expect_ret, client);
			os_timer_arm(&Timer->timer, TIMER_SINGLETIME, 1);
		}
	}
	Timer->tickcount++;
	NODE_DBG("Timer->tickcount is %d",Timer->tickcount);
	//recv refresh data or timeout
	switch(Timer->msgtype){

	NODE_DBG("Timer->msgtype is %d",Timer->msgtype);
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
		if(syn_control_refresh_get()==COMM_REFRESHED || Timer->tickcount>TIMER_TIMEROUT){
			os_timer_disarm(&Timer->timer);
			NODE_DBG("case SYN_CONTROL");
			os_printf("ret tickcount is %d\r\n",Timer->tickcount);
			return send_to_client(client,syn_control_read());
		}
		break;
	case NORMAL_WIFI_SCAN:
		if(Timer->tickcount==1)comm_wifi_api_scan_start();
		else{
			char *ret_jsonstr=(char *)comm_wifi_api_scan();
			if(ret_jsonstr!=NULL){
				os_timer_disarm(&Timer->timer);
				u8 ret=send_to_client(client,ret_jsonstr);
				os_free(ret_jsonstr);
				return ret;
			}else if(Timer->tickcount>TIMER_TIMEROUT*5){
				os_timer_disarm(&Timer->timer);
				send_to_client(client,"error");
			}
		}
		break;
	}
	if(c_sign==CLIENT_IS_HTTP){
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




