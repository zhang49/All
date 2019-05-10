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
#include "config.h"

#include "json/cJson.h"

#include "sensor/ds18b20.h"
#include "sensor/dht22.h"

#include "comm_pub_def.h"
#include "comm_uart.h"
#include "comm_wifi.h"
#include "comm_pwm.h"
#include "comm_light.h"
#include "comm_sensor.h"
#include "comm_relay.h"

#include "mqtt/app.h"
#include "http/app.h"

typedef int (*comm_operator_func)(void *);

typedef struct{
	const char *req_type;
	const char *res_type;
	enum CommMsgType msg_type;
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
		{"GetConnectSynData",	"Reply_GetConnectSynData",	CONNECT_SYN_DATA,			comm_connect_syn,			NULL},
		{"GetWiFiConfig",		"Reply_GetWiFiConfig",		WIFI_CONFIG_READ_REQ,		comm_wifi_config_read,		NULL},
		{"SetWiFiConfig",		"Reply_SetWiFiConfig",		WIFI_CONFIG_WRITE_REQ,		comm_wifi_config_write,		NULL},
		{"SetLigthDuty",		"Reply_SetLigthDuty",		LIGHT_DUTY_VALUE_WRITE_REQ,	comm_led_pwm_duty_write,	NULL},
		{"RayMotorStart",		"Reply_SetMotorStart",		RAY_MOTOR_START_REQ,		comm_ray_motor_start,		NULL},
		{"RayMotorStop",		"Reply_RayMotorStop",		RAY_MOTOR_STOP_REQ,			comm_ray_motor_stop,		NULL},
		{"RayMotorCW",			"Reply_RayMotorCW",			RAY_MOTOR_CW_REQ,			comm_ray_motor_cw,			NULL},
		{"RayMotorCCW",			"Reply_RayMotorCCW",		RAY_MOTOR_CCW_REQ,			comm_ray_motor_ccw,			NULL},
		{"GetRayValue",			"Reply_GetRayValue",		RAY_VALUE_READ_REQ,			comm_ray_value_read,		NULL},
		{"SetRayAlarmValue",	"Reply_SetRayAlarmValue",	RAY_ALARM_VALUE_READ_REQ,	comm_ray_alarm_value_write,	NULL},
		{"GetWiFiScan",			"Reply_GetWiFiScan",		WIFI_SCAN,					comm_expect_ret,			NEEDTIMER},
		{"WiFiConnect",			"Reply_WiFiConnect",		WIFI_CONNECT,				comm_expect_ret,			NEEDTIMER},
		{"Control",				"Reply_Control",			SYN_CONTROL,				comm_expect_ret,			NEEDTIMER},
		{"GetStatus",			"Reply_GetStatus",			SYN_STATE,					comm_syn_status_read,		NULL},
		{NULL,					NULL,						NULL,						NULL,						NULL},

};

os_timer_t doorRequestTimer;
os_timer_t statusTimer;


static config_data *cfg_data=NULL;
uint8_t cfg_save_flag=0;
typedef int(*door_http_send)(http_connection *);
typedef int(*door_mqtt_publish)(MQTT_Client *);

uint8 temperature_read_tick=0;
static void statusTimerCb(void *arg){
	temperature_read_tick++;
	if(temperature_read_tick==8){
		temperature_read_tick=0;
	}
	syn_state.temperature=comm_temperature_value_read_api();
	syn_state.humidity=comm_humidity_value_read_api();
	if(comm_ray_value_api_get()<ray_alarm_value){
		light_alarm_close();	//set hight
	}else{
		light_alarm_open();		//set low
	}
	if(cfg_save_flag){
		config_save(NULL);
		cfg_save_flag=0;
	}
	wifi_connect_check(1000);
	syn_state.run_time++;
}

void ICACHE_FLASH_ATTR comm_init(){
	int ret = config_init();
	cfg_data=config_read();
	if(!ret){
		CONFIG_MAGIC;
		//first use
		wifi_set_opmode(STATIONAP_MODE);
		os_printf("cfg_data->magic!=CONFIG_MAGIC save default config\r\n");
		struct softap_config ap_cfg;
		wifi_softap_get_config(&ap_cfg);
		os_strcpy(ap_cfg.ssid,DEFAULT_AP_SSID);
		ap_cfg.ssid_len=os_strlen(DEFAULT_AP_SSID);
		os_strcpy(ap_cfg.password,DEFAULT_AP_PWD);
		ap_cfg.ssid_hidden=0;
		ap_cfg.authmode=AUTH_OPEN;
		wifi_softap_set_config(&ap_cfg);
		//for test
		struct station_config sta_cfg;
		wifi_station_get_config(&sta_cfg);
		os_strcpy(sta_cfg.ssid,DEFAULT_STA_SSID);
		os_strcpy(sta_cfg.password,DEFAULT_STA_PWD);
		wifi_station_set_config(&sta_cfg);
		os_printf("first use station ssid:%s,pwd:%s\r\n",sta_cfg.ssid,
				sta_cfg.password);
	}
	int mode=wifi_get_opmode();
	if(mode == STATION_MODE || mode == STATIONAP_MODE){
		wifi_station_disconnect();
		wifi_station_connect();
		os_printf("try to connect.\r\n");
	}

	if(ret){
		comm_led_pwm_duty_api_set(cfg_data->light_duty);
		ray_alarm_value=cfg_data->alarm_ray_value;
	}else{
		cfg_data->light_duty = 50;
		cfg_data->alarm_ray_value=100;
		config_save(NULL);
	}

	comm_uart_init();
	os_memset(&doorRequestTimer,0,sizeof(os_timer_t));
	os_timer_disarm(&doorRequestTimer);
	os_timer_setfn(&doorRequestTimer, (os_timer_func_t *)door_request_all_config, NULL);
	os_timer_arm(&doorRequestTimer, 500, 1);

    //ds18b20_init(1);
    comm_pwm_init();
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
	user_data->buf.data = (char*)os_zalloc(len+1);
	user_data->sign=CLIENT_IS_MQTT;
	os_memset(user_data->buf.data,0,len+1);
	os_memcpy(user_data->buf.data, data, len);
	user_data->ptopic=ptopic;
	user_data->pqos=pqos;
	return common_operator_api((void *)user_data);
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



const char *ICACHE_FLASH_ATTR get_res_type(void *client){
	uint8_t c_sign=*((uint8_t *)client);
	session_data_buf *buf=c_sign==CLIENT_IS_HTTP?(((http_connection *)client)->cgi.data):
			&((mqtt_user_data *)client)->buf;
	int index;
	for(index=0;comm_operator[index].req_type!=NULL;index++){
		if(comm_operator[index].msg_type==buf->msgtype){
			return comm_operator[index].res_type;
		}
	}
	return NULL;
}


int ICACHE_FLASH_ATTR send_to_client(void *client,char *message){
	NODE_DBG("send_to_client....");
	if(*(uint8_t *)client==CLIENT_IS_MQTT){
		mqtt_user_data *mqtt_data=(mqtt_user_data *)client;
		NODE_DBG("send_to_client public.");
		mqtt_publish_api(mqtt_data->ptopic,message,os_strlen(message),mqtt_data->pqos,0);
		if(mqtt_data->buf.data!=NULL)
			os_free(mqtt_data->buf.data);
		if(mqtt_data!=NULL)
			os_free(mqtt_data);
	}else if(*(uint8_t *)client==CLIENT_IS_HTTP){
		http_write(client,message);
		if(((session_data_buf *)((http_connection *)client)->cgi.data)->data!=NULL)
			os_free(((session_data_buf *)((http_connection *)client)->cgi.data)->data);
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
	cJSON *root = cJSON_Parse(client_sign==CLIENT_IS_HTTP?(((http_connection *)client)->body.data):
			(((mqtt_user_data *)client)->buf.data));
	cJSON *root_data = cJSON_GetObjectItem(root,"data");
	if(root_data==NULL){
		error_code=EC_None;
	}else{
		comm_wifi_config_write_api(root_data);
	}
	cJSON_Delete(root);
	return send_ret_json(client,retroot,error_code);
}


int ICACHE_FLASH_ATTR comm_led_pwm_duty_write(void *client){
	uint8 client_sign=*(uint8_t *)client;
	enum ErrorCode error_code=EC_Normal;
	cJSON *retroot=cJSON_CreateObject();
	cJSON *ret_data;
	cJSON *root = cJSON_Parse(client_sign==CLIENT_IS_HTTP?(((http_connection *)client)->body.data):
			(((mqtt_user_data *)client)->buf.data));
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
	cfg_data->light_duty=data_duty->valueint;
	cfg_save_flag=1;
	cJSON_AddItemToObject(retroot,"data",ret_data=cJSON_CreateObject());
	cJSON_AddNumberToObject(ret_data,"duty",comm_led_pwm_duty_api_get());
	//must limits
badJson:
	if(root!=NULL){
		cJSON_Delete(root);
	}
	return send_ret_json(client,retroot,error_code);
}

int ICACHE_FLASH_ATTR comm_ray_motor_cw(void *client){
	cJSON *retroot=cJSON_CreateObject();
	motor_move_espnow_write(3,65535,1);
	return send_ret_json(client,retroot,EC_Normal);

}
int ICACHE_FLASH_ATTR comm_ray_motor_ccw(void *client){
	cJSON *retroot=cJSON_CreateObject();
	motor_move_espnow_write(3,65535,2);
	return send_ret_json(client,retroot,EC_Normal);
}
int ICACHE_FLASH_ATTR comm_ray_motor_stop(void *client){
	cJSON *retroot=cJSON_CreateObject();
	motor_move_espnow_write(255,255,0);
	return send_ret_json(client,retroot,EC_Normal);
}
int ICACHE_FLASH_ATTR comm_ray_motor_start(void *client){
	char *data;
		uint8 client_sign=*(uint8_t *)client;
		http_connection *http_client;
		if(client_sign==CLIENT_IS_MQTT){
			mqtt_user_data *user_data=(mqtt_user_data *)client;
			data = user_data->buf.data;
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
		cJSON *speed=cJSON_GetObjectItem(c_data,"speed");
		cJSON *duration=cJSON_GetObjectItem(c_data,"duration");
		cJSON *direction=cJSON_GetObjectItem(c_data,"direction");
		if(!speed || !duration || !direction){
			error_code=EC_None;
			goto badJson;
		}
		//cJSON_Delete(root);
		motor_move_espnow_write(speed->valueint,duration->valueint,direction->valueint);
		cJSON *retdata;
		cJSON_AddItemToObject(retroot,"data", retdata = cJSON_CreateObject());
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
int ICACHE_FLASH_ATTR comm_ray_alarm_value_write(void *client){
	char *data;
	uint8 client_sign=*(uint8_t *)client;
	http_connection *http_client;
	if(client_sign==CLIENT_IS_MQTT){
		mqtt_user_data *user_data=(mqtt_user_data *)client;
		data = user_data->buf.data;
	}else if(client_sign==CLIENT_IS_HTTP){
		http_client=(http_connection *)client;
		data=(client_sign==CLIENT_IS_HTTP?(((http_connection *)client)->body.data):
				(((mqtt_user_data *)client)->buf.data));
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
	cfg_data->alarm_ray_value=ray_alarm_value;
	cfg_save_flag=1;
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


int ICACHE_FLASH_ATTR common_operator_api(void *client){
	comm_positive++;
	char *data;
	mqtt_user_data *mqtt_data;
	uint8_t client_sign=*(uint8_t *)client;
	http_connection *http_client;
	if(client_sign==CLIENT_IS_MQTT){
		mqtt_data=(mqtt_user_data *)client;
		data = mqtt_data->buf.data;
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
	if(os_strcmp(type->valuestring,"GetStatus")){
		os_printf("request type :%s\r\n",type->valuestring);
	}
	int index,ret;
	int type_is_exist=0;
	for(index=0;comm_operator[index].req_type!=NULL;index++){
		if(strcmp(type->valuestring,comm_operator[index].req_type)==0){
			type_is_exist=1;

			session_data_buf *sion_buf;
			if(client_sign==CLIENT_IS_HTTP){
				//set header
				http_SET_HEADER(client,HTTP_CONTENT_TYPE,JSON_CONTENT_TYPE);
				http_response_OK(client);
				//create memory for http session
				sion_buf=(session_data_buf *)os_zalloc(sizeof(session_data_buf));
				http_client->cgi.data=sion_buf;
			}else{
				sion_buf=&mqtt_data->buf;
			}

			sion_buf->msgtype=comm_operator[index].msg_type;

			if(comm_operator[index].flag==NULL){
				//Nothing to do
			}else if(comm_operator[index].flag==NEEDTIMER){
				if(client_sign==CLIENT_IS_HTTP){
					sion_buf->data=(char *)os_zalloc(os_strlen(http_client->body.data)+1);
					os_strcpy(sion_buf->data,http_client->body.data);
					http_client->cgi.function=(http_callback)comm_operator[index].func;
				}
				sion_buf->tickcount=0;
				//send to Master
				switch(comm_operator[index].msg_type){
					case NORMAL_CONFIG:

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
						}while(0);
						break;
					case WIFI_SCAN:
						NODE_DBG("msg type is:NORMAL_WIFI_SCAN");

						break;
				}
			}
			if(root){
				cJSON_Delete(root);
				root=NULL;
			}
			return comm_operator[index].func(client);
		}
	}
	if(root){
		cJSON_Delete(root);
		root=NULL;
	}
	if(type_is_exist==0){
		os_printf("didn't has this type.\r\n");
		return send_to_client(client,"error");
	}

badJson:
os_printf("badJson.\r\n");
	if(root!=NULL){
		cJSON_Delete(root);
		root=NULL;
	}
	if(client_sign==CLIENT_IS_HTTP){
		http_response_BAD_REQUEST(client);
		return HTTPD_CGI_DONE;
	}
	if(client_sign==CLIENT_IS_MQTT){
		//return send_ret_json(client,cJSON_CreateObject(),EC_Unknown);
		if(mqtt_data->buf.data!=NULL)os_free(mqtt_data->buf.data);
		if(mqtt_data!=NULL)os_free(mqtt_data);
	}
	return 1;
}


int ICACHE_FLASH_ATTR comm_expect_ret(void *client){
	session_data_buf *sion_buf;
	uint8 c_sign=*(uint8_t *)client;
	sion_buf=c_sign==CLIENT_IS_HTTP?(((http_connection *)client)->cgi.data):
							&((mqtt_user_data *)client)->buf;

		if(sion_buf->tickcount==0){
			os_memset(&sion_buf->timer,0,sizeof(os_timer_t));
			os_timer_disarm(&sion_buf->timer);
			if(c_sign==CLIENT_IS_HTTP){
				os_timer_setfn(&sion_buf->timer, http_execute_cgi, client);
			}else{
				os_timer_setfn(&sion_buf->timer, comm_expect_ret, client);
			}

			os_timer_arm(&sion_buf->timer, COMM_TIMER_SINGLE_TIME, 1);
		}
		sion_buf->tickcount += COMM_TIMER_SINGLE_TIME;

	//NODE_DBG("Timer->tickcount is %d",Timer->tickcount);
	//recv refresh data or timeout
	switch(sion_buf->msgtype){
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
			char *data=(c_sign==CLIENT_IS_HTTP?(((session_data_buf *)(((http_connection *)client)->cgi.data))->data):
					(((mqtt_user_data *)client)->buf.data));
			cJSON *root=cJSON_Parse(data);
			cJSON *r_data=cJSON_GetObjectItem(root,"data");
			uint8 index=cJSON_GetObjectItem(r_data,"index")->valueint;
			if(comm_relay_refresh_status_get(index)==COMM_REFRESHED || sion_buf->tickcount>COMM_TIMER_TIMEOUT){
				os_timer_disarm(&sion_buf->timer);
				cJSON *retroot=cJSON_CreateObject();
				NODE_DBG("SYN_CONTROL Ret: %s",(comm_relay_refresh_status_get(index)==COMM_REFRESHED?"Refreshed":"Timer timeout"));
				enum ErrorCode error_code = EC_Failed;
				uint8 d_type=cJSON_GetObjectItem(r_data,"device_type")->valueint;
				uint8 op=cJSON_GetObjectItem(r_data,"op")->valueint;
				cJSON *ret_data;
				cJSON_AddItemToObject(retroot,"data",ret_data = cJSON_CreateObject());
				cJSON_AddNumberToObject(ret_data,"device_type",d_type);
				cJSON_AddNumberToObject(ret_data,"index",index);
				cJSON_AddNumberToObject(ret_data,"op",op);
				if(comm_relay_refresh_status_get(index)==COMM_REFRESHED){
					error_code=EC_Normal;
				}
				cJSON_Delete(root);
				return send_ret_json(client,retroot,error_code);
			}
			cJSON_Delete(root);
		}
		break;

	case WIFI_CONNECT:
		if(sion_buf->tickcount==COMM_TIMER_SINGLE_TIME){
			char *data=(c_sign==CLIENT_IS_HTTP?(((session_data_buf *)(((http_connection *)client)->cgi.data))->data):
					(((mqtt_user_data *)client)->buf.data));
			cJSON *root=cJSON_Parse(data);
			cJSON *root_data = cJSON_GetObjectItem(root,"data");
			char *ssid = cJSON_GetObjectItem(root_data,"wifi_station_ssid")->valuestring;
			char *pwd = cJSON_GetObjectItem(root_data,"wifi_station_pwd")->valuestring;
			if(!comm_wifi_start_connect_ap_api(ssid,pwd)){
				os_timer_disarm(&sion_buf->timer);
				send_ret_json(client,cJSON_CreateObject(),EC_Busy);
			}
			cJSON_Delete(root);
		}else{
			if(sion_buf->tickcount>COMM_TIMER_TIMEOUT && comm_wifi_connect_ap_check_api()){
				os_timer_disarm(&sion_buf->timer);
				cJSON *retroot=cJSON_CreateObject();
				return send_ret_json(client,retroot,EC_Normal);
			}
			if(sion_buf->tickcount>COMM_TIMER_TIMEOUT * 5){
				os_timer_disarm(&sion_buf->timer);
				//connect failed, connect default ap
				comm_wifi_connect_default_ap_api();
				cJSON *retroot=cJSON_CreateObject();
				return send_ret_json(client,retroot,EC_Failed);
			}
		}
		break;
	case WIFI_SCAN:
		if(sion_buf->tickcount==COMM_TIMER_SINGLE_TIME){
			if(!comm_wifi_scan_start_api()){
				os_timer_disarm(&sion_buf->timer);
				return send_ret_json(client,cJSON_CreateObject(),EC_Busy);
			}
		}
		else{
			cJSON *retroot=(cJSON *)comm_wifi_scan_api();
			if(retroot!=NULL){
				os_timer_disarm(&sion_buf->timer);
				return send_ret_json(client,retroot,EC_Normal);
			}
			else if(sion_buf->tickcount>COMM_TIMER_TIMEOUT * 3){
				os_timer_disarm(&sion_buf->timer);
				retroot=cJSON_CreateObject();
				return send_ret_json(client,retroot,EC_Failed);
			}
		}
		break;
	}
	if(c_sign==CLIENT_IS_HTTP){
		return HTTPD_CGI_MORE;
	}
	return 1;
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

	return send_to_client(client,"error");
}




