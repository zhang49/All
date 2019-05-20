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
#include "mqtt/aliyun_mqtt.h"
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
		{"GetConnectSynData",		CONNECT_SYN_DATA,			comm_connect_syn,				NULL},
		{"GetStatus",				SYN_STATE,					comm_syn_status_read,			NULL},
		{"GetWiFiInfo",				WIFI_CONFIG_READ_REQ,		comm_wifi_config_read,			NULL},
		{"SetWiFiApConfig",			WIFI_CONFIG_WRITE_REQ,		comm_wifi_ap_config_write,		NULL},
		{"SetLightLuminance",		LIGHT_DUTY_VALUE_WRITE_REQ,	comm_led_pwm_duty_write,		NULL},
		{"RayMotorStart",			RAY_MOTOR_START_REQ,		comm_ray_motor_start,			NULL},
		{"RayMotorStop",			RAY_MOTOR_STOP_REQ,			comm_ray_motor_stop,			NULL},
		{"RayMotorCW",				RAY_MOTOR_CW_REQ,			comm_ray_motor_cw,				NULL},
		{"RayMotorCCW",				RAY_MOTOR_CCW_REQ,			comm_ray_motor_ccw,				NULL},
		//{"GetLightLux",				RAY_VALUE_READ_REQ,			comm_ray_value_read,			NULL},
		{"SetLightLuxAlarmValue",	RAY_ALARM_VALUE_READ_REQ,	comm_ray_alarm_value_write,		NULL},
		{"GetWiFiScan",				WIFI_SCAN,					comm_expect_ret,				NEEDTIMER},
		{"WiFiConnect",				WIFI_CONNECT,				comm_expect_ret,				NEEDTIMER},
		{"Control",					SYN_CONTROL,				comm_expect_ret,				NEEDTIMER},

		{NULL,						NULL,						NULL,							NULL},

};

os_timer_t statusTimer;
int mqtt_syn_flag=0;

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
	if(mqtt_is_connected()){
		if(temperature_read_tick==5){
			comm_syn_status_read(NULL);
		}
		if(mqtt_syn_flag==0){
			comm_connect_syn(NULL);
			mqtt_syn_flag=1;
		}
	}else{
		mqtt_syn_flag=0;
	}
}

mqtt_user_data *ICACHE_FLASH_ATTR create_mqtt_user_data(char *tpoic){
	mqtt_user_data *mud=(mqtt_user_data *)os_malloc(sizeof(mqtt_user_data));
	mud->sign=CLIENT_IS_MQTT;
	mud->buf.data=NULL;
	os_strcpy(mud->topic,tpoic);
	return mud;
}

int ICACHE_FLASH_ATTR mqtt_send_wifiscan(){
	cJSON *wifi_scan=(cJSON *)comm_wifi_scan_api();
	if(wifi_scan==NULL){
		return 1;
	}
	char *head_form = "\"Ap\":";
	cJSON *retroot = cJSON_CreateObject();
	cJSON *arr,*params;
	cJSON_AddStringToObject(retroot,"method", "thing.event.WiFiScan.post");
	cJSON_AddStringToObject(retroot, "version", "1.0.0");
	cJSON_AddItemToObject(retroot, "params",params = cJSON_CreateObject());
	cJSON_AddItemToObject(params,"Ap",wifi_scan);
	char *json_str = cJSON_PrintUnformatted(retroot);
	json_str=replace_str_for_mqtt_transmit(json_str,"\"Ap\":");
	mqtt_publish_api(WIFI_SCAN_TOPIC,json_str,os_strlen(json_str),0,0);
}

/*
 * replace " to \" before headStr after, stop while a whole object
 */
char *ICACHE_FLASH_ATTR replace_str_for_mqtt_transmit(char *destStr,char *headStr){
	char *head,*thead;
	int rep_count;
	rep_count = 0;
	head= thead=NULL;
	head = (char *)os_strstr(destStr, headStr);
	if (!head)return 0;
	thead = head;
	int left_include = 0;
	int tt=0;
	thead += os_strlen(headStr);
	while (*thead != 0) {
		if (*thead == '{') {
			left_include++;
		}
		else if (*thead == '}') {
			left_include--;
			if (left_include == 0)break;
		}
		if (*thead == '\"') {
			rep_count += 1;
		}
		thead++;
	}
	// destStr ,'"' count, head {, tail } and 1 \0 memery
	char *temp = (char *)os_malloc(os_strlen(destStr) + rep_count + 2 + 1);
	int i;
	int index = os_strlen(destStr) - os_strlen(head) + os_strlen(headStr);
	//os_printf("count is:%d,index:%d,left_include:%d\r\n", count, index,left_include);
	for (i = 0; i < index; i++) {
		temp[i] = destStr[i];
	}
	head = destStr + index;
	tt = left_include;
	while (*head != 0) {
		if (*head == '{') {
			if (left_include==0) {
				temp[index++] = '"';
			}
			left_include++;
		}
		else if (*head == '}'){
			left_include--;
			if (left_include == 0) {
				temp[index++] = *head++;
				temp[index++] = '"';
				break;
			}
		}
		if (*head == '\"') {
			temp[index++] = '\\';
		}
		temp[index++] = *head;
		head++;
	}
	for (; *head != 0;) {
		temp[index++] = *head++;
	}
	temp[index] = 0;
	os_free(destStr);
	return temp;
}

void ICACHE_FLASH_ATTR comm_init(){
	//for test!
	comm_relay_status_set_api(0,1);
	comm_relay_status_set_api(1,1);
	comm_temperature_value_write_api(1111);
	comm_humidity_value_write_api(2222);
	comm_SoilMoisture_value_write_api(3333);
	comm_ray_value_write_api(255);
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
	if(!ret){
		cfg_data->LightLuminance = 50;
		cfg_data->LightLuxAlarmValue=100;
		config_save(NULL);
	}
	comm_led_pwm_duty_api_set(cfg_data->LightLuminance);
	ray_alarm_value=cfg_data->LightLuxAlarmValue;

	comm_uart_init();

    //ds18b20_init(1);
    comm_pwm_init();
    comm_relay_init();
    comm_sensor_init();
    comm_light_init();


    os_memset(&statusTimer,0,sizeof(os_timer_t));
    os_timer_disarm(&statusTimer);
    os_timer_setfn(&statusTimer, (os_timer_func_t *)statusTimerCb, NULL);
    os_timer_arm(&statusTimer, 1000, 1);
    comm_wifi_scan_start_api();
    comm_wifi_scan_api();
}

int ICACHE_FLASH_ATTR http_operator_api(http_connection *c){
	//wait for whole body
	if(c->state < HTTPD_STATE_BODY_END)
		return HTTPD_CGI_MORE;
	c->sign=CLIENT_IS_HTTP;
	return common_operator_api(c);
}


int ICACHE_FLASH_ATTR mqtt_operator_api(char *topic,const char* data,uint32 len){
	mqtt_user_data *user_data;
	//must copy
	user_data=(mqtt_user_data *)os_zalloc(sizeof(mqtt_user_data));
	user_data->buf.data = (char*)os_zalloc(len+1);
	user_data->sign=CLIENT_IS_MQTT;
	os_strcpy(user_data->topic,topic);
	os_memset(user_data->buf.data,0,len+1);
	os_memcpy(user_data->buf.data, data, len);
	return common_operator_api((void *)user_data);
}

/*
 * http send string and free string
 */
uint8 ICACHE_FLASH_ATTR http_send_string(void *client,char *str){
	http_write(client,str);
	session_data_buf *buf=(session_data_buf *)((http_connection *)client)->cgi.data;
	if(buf->data!=NULL)
		os_free(buf->data);
	if(((http_connection *)client)->cgi.data!=NULL)
		os_free(((http_connection *)client)->cgi.data);
	os_free(str);
	return HTTPD_CGI_DONE;
}

uint8_t ICACHE_FLASH_ATTR mqtt_upload_device_property(cJSON *retroot){
	char *ostream=cJSON_PrintUnformatted(retroot);
	cJSON_Delete(retroot);
	mqtt_publish_api(DEVICE_PROPERTY_TOPIC,ostream,os_strlen(ostream),0,0);
	os_free(ostream);
	return 1;
}

/*
 * mqtt send string and free string
 */
uint8 ICACHE_FLASH_ATTR mqtt_send_string(void *client,char *str){
	mqtt_user_data *mqtt_data=(mqtt_user_data *)client;
	NODE_DBG("send_to_client public.");
	const char *topic=NULL;
	if(mqtt_data->topic!=NULL){
		topic=mqtt_data->topic;
	}else{
		os_printf("topic is NULL");
		return 0;
	}
	if(str==NULL || os_strlen(str)==0){
		os_printf("mqtt send string is zero.\r\n");
	}
	mqtt_publish_api(topic,str,os_strlen(str),0,0);
	if(mqtt_data->buf.data!=NULL){
		os_free(mqtt_data->buf.data);
	}
	if(mqtt_data!=NULL){
		os_free(mqtt_data);
	}
	os_free(str);
	return 1;
}

uint8 ICACHE_FLASH_ATTR send_ret_json(void *client,cJSON *retroot,enum ErrorCode error_code){
	NODE_DBG("send to client....");
	int ret=1;
	cJSON_AddNumberToObject(retroot, "error_code",error_code );
	int i;
	for(i=0;i<ErrorCodeSize;i++){
		if(error_msg[i].error_code==error_code){
			cJSON_AddStringToObject(retroot, "error_str",error_msg[i].error_str);
			break;
		}
	}
	char *ostream=cJSON_PrintUnformatted(retroot);
	cJSON_Delete(retroot);
	if(*(uint8_t *)client==CLIENT_IS_HTTP){
		ret=http_send_string(client,ostream);
	}else{
		ret=mqtt_send_string(client,ostream);
	}
	return ret;
}

int ICACHE_FLASH_ATTR comm_connect_syn(void *client){
	cJSON *wifi_cfg=comm_wifi_config_read_api();
	cJSON *data;
	cJSON *retroot=cJSON_CreateObject();
	uint8_t c_sign;
	if(client==NULL){
		c_sign=CLIENT_IS_MQTT;
	}else{
		c_sign=*((uint8_t *)client);
	}
	if(c_sign==CLIENT_IS_MQTT){
		cJSON_AddStringToObject(retroot,"method","thing.event.property.post");
		cJSON_AddStringToObject(retroot,"id","1");
		cJSON_AddItemToObject(retroot,"params",data=cJSON_CreateObject());
		cJSON_AddStringToObject(retroot,"version","1.0");
	}else{
		cJSON_AddItemToObject(retroot,"data",data=cJSON_CreateObject());
	}
	cJSON_AddStringToObject(data,"StationSSID",cJSON_GetObjectItem(wifi_cfg
			,"StationSSID")->valuestring);
	cJSON_AddStringToObject(data,"ApSSID",cJSON_GetObjectItem(wifi_cfg
			,"ApSSID")->valuestring);
	cJSON_AddStringToObject(data,"ConnectSSID",cJSON_GetObjectItem(wifi_cfg
			,"ConnectSSID")->valuestring);
	cJSON_AddStringToObject(data,"MACAddress",cJSON_GetObjectItem(wifi_cfg
			,"MACAddress")->valuestring);
	cJSON_AddNumberToObject(data,"E_Craft_Speed_Motor_Punch",255);
	cJSON_AddNumberToObject(data,"LightLuxAlarmValue",ray_alarm_value);
	//free
	cJSON_Delete(wifi_cfg);
	if(client==NULL){
		return mqtt_upload_device_property(retroot);
	}
	return send_ret_json(client,retroot,EC_Normal);
}

int ICACHE_FLASH_ATTR comm_syn_status_read(void *client){
	int ret=HTTPD_CGI_DONE;
	cJSON *data;
	cJSON *retroot=cJSON_CreateObject();
	uint8_t c_sign;
	if(client==NULL){
		c_sign=CLIENT_IS_MQTT;
	}else{
		c_sign=*((uint8_t *)client);
	}
	if(c_sign==CLIENT_IS_MQTT){
		cJSON_AddStringToObject(retroot,"method","thing.event.property.post");
		cJSON_AddStringToObject(retroot,"id","1");
		cJSON_AddItemToObject(retroot,"params",data=cJSON_CreateObject());
		cJSON_AddStringToObject(retroot,"version","1.0");
	}else{
		cJSON_AddItemToObject(retroot,"data",data=cJSON_CreateObject());
	}
	cJSON_AddNumberToObject(data,"Runtime",syn_state.run_time);
	cJSON_AddNumberToObject(data,"CurrentTemperature",comm_temperature_value_read_api());
	cJSON_AddNumberToObject(data,"CurrentHumidity",comm_humidity_value_read_api());
	cJSON_AddNumberToObject(data,"SoilMoisture",comm_SoilMoisture_value_read_api());
	cJSON_AddNumberToObject(data,"LightLuminance",comm_led_pwm_duty_api_get());
	cJSON_AddNumberToObject(data,"LightLux",comm_ray_value_api_get());
	cJSON *relay_arr;
	cJSON_AddItemToObject(data,"Relay",relay_arr=cJSON_CreateArray());
	int i;
	for(i=0;i<6;i++){
		if(i<RELAYSIZE){
			cJSON_AddNumberToObject(relay_arr,"",comm_relay_status_get_api(i));
		}
		else{
			cJSON_AddNumberToObject(relay_arr,"",-1);
		}
	}
	if(client==NULL){
		return mqtt_upload_device_property(retroot);
	}
	return send_ret_json(client,retroot,EC_Normal);
}



int ICACHE_FLASH_ATTR comm_wifi_config_read(void *client){
	cJSON *retroot=(cJSON *)comm_wifi_config_read_api();
	return send_ret_json(client,retroot,EC_Normal);
}

int ICACHE_FLASH_ATTR comm_wifi_ap_config_write(void *client){
	uint8 client_sign=*(int *)client;
	cJSON *retroot=cJSON_CreateObject();
	enum ErrorCode error_code=EC_Normal;
	cJSON *root = cJSON_Parse(client_sign==CLIENT_IS_HTTP?(((http_connection *)client)->body.data):
			(((mqtt_user_data *)client)->buf.data));
	cJSON *root_data = cJSON_GetObjectItem(root,"data");
	if(root_data==NULL){
		error_code=EC_None;
	}else if(!comm_wifi_ap_config_write_api(root_data)){
		error_code = EC_Failed;
	};
	cJSON_Delete(root);
	//resend syn connect data
	mqtt_syn_flag=0;
	return send_ret_json(client,retroot,error_code);
}


int ICACHE_FLASH_ATTR comm_led_pwm_duty_write(void *client){
	uint8 c_sign=*(uint8_t *)client;
	enum ErrorCode error_code=EC_Normal;
	cJSON *retroot=cJSON_CreateObject();
	cJSON *ret_data;
	cJSON *root = cJSON_Parse(c_sign==CLIENT_IS_HTTP?(((http_connection *)client)->body.data):
			(((mqtt_user_data *)client)->buf.data));
	cJSON *root_data = cJSON_GetObjectItem(root,"data");
	cJSON *data_duty;
	if(root_data==NULL || (data_duty=cJSON_GetObjectItem(root_data,"LightLuminance"))==NULL){
		error_code=EC_Failed;
		goto badJson;
	}
	comm_led_pwm_duty_api_set(data_duty->valueint);
	cfg_data->LightLuminance=data_duty->valueint;
	cfg_save_flag=1;
	if(c_sign==CLIENT_IS_HTTP){
		cJSON_AddItemToObject(retroot,"data",ret_data=cJSON_CreateObject());
	}else{
		ret_data=retroot;
	}
	cJSON_AddNumberToObject(ret_data,"LightLuminance",comm_led_pwm_duty_api_get());
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
	cJSON_AddNumberToObject(data, "LightLuxAlarmValue",comm_ray_value_api_get() );
	return send_ret_json(client,retroot,EC_Normal);

}

int ICACHE_FLASH_ATTR comm_ray_alarm_value_write(void *client){
	char *data;
	uint8 c_sign=*(uint8_t *)client;
	cJSON *retroot=cJSON_CreateObject();
	enum ErrorCode error_code=EC_Normal;
	cJSON *root = cJSON_Parse(c_sign==CLIENT_IS_HTTP?(((http_connection *)client)->body.data):
			(((mqtt_user_data *)client)->buf.data));
	cJSON *r_data,*data_rv;
	r_data = cJSON_GetObjectItem(root,"data");
	if(r_data==NULL || (data_rv=cJSON_GetObjectItem(r_data,"LightLuxAlarmValue"))==NULL){
		error_code=EC_Failed;
		goto badJson;
	}
	ray_alarm_value=data_rv->valueint;
	cfg_data->LightLuxAlarmValue=ray_alarm_value;
	cfg_save_flag=1;
	cJSON *ret_data;
	if(c_sign==CLIENT_IS_HTTP){
		cJSON_AddItemToObject(retroot,"data", ret_data = cJSON_CreateObject());
	}else{
		ret_data=retroot;
	}
	cJSON_AddNumberToObject(ret_data,"LightLuxAlarmValue", ray_alarm_value);
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
				switch(comm_operator[index].msg_type){
				case CONNECT_SYN_DATA:
				case SYN_STATE:
					//os_strcpy(mqtt_data->topic,DEVICE_PROPERTY_TOPIC);
					break;
				case WIFI_CONFIG_READ_REQ:
				case WIFI_CONFIG_WRITE_REQ:
				case LIGHT_DUTY_VALUE_WRITE_REQ:
				case RAY_MOTOR_START_REQ:
				case RAY_MOTOR_STOP_REQ:
				case RAY_MOTOR_CW_REQ:
				case RAY_MOTOR_CCW_REQ:
				case RAY_VALUE_READ_REQ:
				case RAY_ALARM_VALUE_READ_REQ:

					break;
				case WIFI_SCAN:
					//os_strcpy(mqtt_data->topic,WIFI_SCAN_TOPIC);
					break;
				case WIFI_CONNECT:
					//os_strcpy(mqtt_data->topic,WIFI_CONNECT_TOPIC);
					break;
				case SYN_CONTROL:
					break;
				}
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
							cJSON *d_index=cJSON_GetObjectItem(c_data,"index");
							cJSON *d_status=cJSON_GetObjectItem(c_data,"status");
							if(d_index==NULL || d_status==NULL){
								goto badJson;
							}
							comm_relay_refresh_set(index,COMM_NOREFRESH);
							comm_relay_status_set_app_api(d_index->valueint,d_status->valueint);
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
		return 1;//send_to_client(client,"error");
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
			{
				//target for test
				uint8 status=cJSON_GetObjectItem(r_data,"status")->valueint;
				comm_relay_status_set_api(index,status);
				comm_relay_refresh_set(index,COMM_REFRESHED);
			}
			if(comm_relay_refresh_status_get(index)==COMM_REFRESHED || sion_buf->tickcount>COMM_TIMER_TIMEOUT){
				os_timer_disarm(&sion_buf->timer);
				cJSON *retroot=cJSON_CreateObject();
				//os_printf("SYN_CONTROL Ret: %s",(comm_relay_refresh_status_get(index)==COMM_REFRESHED?"Refreshed":"Timer timeout"));
				enum ErrorCode error_code = EC_Failed;
				if(comm_relay_refresh_status_get(index)==COMM_REFRESHED){
					error_code=EC_Normal;
				}
				//uint8 d_type=cJSON_GetObjectItem(r_data,"device_type")->valueint;
				//uint8 status=cJSON_GetObjectItem(r_data,"status")->valueint;
				cJSON *ret_data;
				cJSON_AddItemToObject(retroot,"data",ret_data = cJSON_CreateObject());
				int i;
				cJSON *relay_arr;
				cJSON_AddItemToObject(ret_data,"Relay",relay_arr=cJSON_CreateArray());
				for(i=0;i<6;i++){
					if(i<RELAYSIZE){
						cJSON_AddNumberToObject(relay_arr,"",comm_relay_status_get_api(i));
					}
					else{
						cJSON_AddNumberToObject(relay_arr,"",-1);
					}
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
			cJSON *d_ssid = cJSON_GetObjectItem(root_data,"wifi_station_ssid");
			if(d_ssid==NULL){
				os_timer_disarm(&sion_buf->timer);
				cJSON_Delete(root);
				send_ret_json(client,cJSON_CreateObject(),EC_Failed);
			}
			cJSON *d_pwd = cJSON_GetObjectItem(root_data,"wifi_station_pwd");
			if(comm_wifi_start_connect_ap_api(d_ssid->valuestring,d_pwd->valuestring)==0){
				os_timer_disarm(&sion_buf->timer);
				cJSON_Delete(root);
				return send_ret_json(client,cJSON_CreateObject(),EC_Busy);
			}
			cJSON_Delete(root);
		}else{
			if(sion_buf->tickcount>COMM_TIMER_TIMEOUT && comm_wifi_connect_ap_check_api()){
				os_timer_disarm(&sion_buf->timer);
				cJSON *retroot=cJSON_CreateObject();
				//resend syn connect data
				mqtt_syn_flag=0;
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
			cJSON *ws_json=(cJSON *)comm_wifi_scan_api();
			if(ws_json!=NULL){
				cJSON *retroot=cJSON_CreateObject();
				os_timer_disarm(&sion_buf->timer);
				{
					cJSON *data;
					//mqtt send data to aliyun, string need hanle
					//						char *head_form = "\"Ap\":";
					cJSON *retroot = cJSON_CreateObject();
					//						cJSON_AddStringToObject(retroot,"method", "thing.event.WiFiScan.post");
					//						cJSON_AddStringToObject(retroot, "version", "1.0.0");
					//						cJSON_AddItemToObject(retroot, "params",data = cJSON_CreateObject());
					//						cJSON_AddItemToObject(retroot,"data",data=cJSON_CreateObject());
					cJSON_AddItemToObject(retroot,"data",ws_json);
					//						char *json_str = cJSON_PrintUnformatted(retroot);
					//						cJSON_Delete(retroot);
					//						json_str=replace_str_for_mqtt_transmit(json_str,"\"Ap\":");
					//						mqtt_send_string(client,json_str);
					return send_ret_json(client,retroot,EC_Normal);

				}
			}
			else if(sion_buf->tickcount>COMM_TIMER_TIMEOUT * 3){
				os_timer_disarm(&sion_buf->timer);
				ws_json=cJSON_CreateObject();
				return send_ret_json(client,ws_json,EC_Failed);
			}
		}
		break;
	}
	if(c_sign==CLIENT_IS_HTTP){
		return HTTPD_CGI_MORE;
	}
	return 1;
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





