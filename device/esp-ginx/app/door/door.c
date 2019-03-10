/*
 * door.c
 *
 *  Created on: Feb 26, 2019
 *      Author: root
 */

#include "door.h"

#include "door_uart.h"

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

#include "door/door_uart.h"
#include "http/app.h"




typedef int (*door_operator_func)(void *);

typedef struct{
	const char *type;
	door_operator_func func;
	int flag;
}door_def;


static door_def door_operator[]={
		{"GetWiFiConfig",	door_wifi_config_read,		NULL},
		{"GetDoorConfig",	door_door_config_read,		NULL},
		{"GetSafeConfig",	door_safe_config_read,		NULL},
		{"SetWiFiConfig",	door_wifi_config_write,		NULL},
		{"SetSafeConfig",	door_door_expect_ret,		NEEDTIMER},
		{"SetDoorConfig",	door_door_expect_ret,		NEEDTIMER},
		{"Command",			door_door_expect_ret,		NEEDTIMER},
		{NULL,				NULL,						NULL}
};
uint8 remarkId_set = 0;

extern door_conf_buf configbuf;

os_timer_t doorRequestTimer;

typedef int(*door_http_send)(http_connection *);
typedef int(*door_mqtt_publish)(MQTT_Client *);

void ICACHE_FLASH_ATTR door_init(){
	door_uart_init();
	os_memset(&doorRequestTimer,0,sizeof(os_timer_t));
	os_timer_disarm(&doorRequestTimer);
	os_timer_setfn(&doorRequestTimer, (os_timer_func_t *)door_request_all_config, NULL);
	os_timer_arm(&doorRequestTimer, 500, 1);
}


int ICACHE_FLASH_ATTR http_door_operator_api(http_connection *c){
	//wait for whole body
	if(c->state < HTTPD_STATE_BODY_END)
		return HTTPD_CGI_MORE;
	c->sign=CLIENT_IS_HTTP;
	return door_operator_api(c);
}
int ICACHE_FLASH_ATTR door_operator_api(void *client){
	char *data;
	uint8 client_sign=*(int *)client;
	http_connection *http_client;
	mqtt_door_comm_data *mqtt_user_data;
	if(client_sign==CLIENT_IS_MQTT){
		MQTT_Client *mqtt_client=(MQTT_Client *)client;
		//malloc??????
		mqtt_user_data=(mqtt_door_comm_data *)mqtt_client->user_data;
		//must copy
		data = (char*)os_zalloc(mqtt_user_data->data_len+1);
		os_memcpy(data, mqtt_user_data->msg, mqtt_user_data->data_len);
		data[mqtt_user_data->data_len] = 0;
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

	int index;
	int type_is_exist=0;
	for(index=0;door_operator[index].type!=NULL;index++){
		if(strcmp(type->valuestring,door_operator[index].type)==0){
			cJSON_Delete(root);
			type_is_exist=1;
			if(door_operator[index].flag==NULL){
				if(client_sign==CLIENT_IS_HTTP){
					//set header
					http_SET_HEADER(client,HTTP_CONTENT_TYPE,JSON_CONTENT_TYPE);
					http_response_OK(client);
				}
			}else if(door_operator[index].flag==NEEDTIMER){
				remarkId_set++;
				send_message_to_master(remarkId_set,0x02,data,os_strlen(data));
				client_handle_timer *Timer;
				if(client_sign==CLIENT_IS_MQTT){
					os_free(data);
					Timer=&mqtt_user_data->timer;
				}else if(client_sign==CLIENT_IS_HTTP){
					Timer=(client_handle_timer *)os_malloc(sizeof(client_handle_timer));
					http_client->cgi.data=Timer;
					http_client->cgi.function=(http_callback)door_operator[index].func;

				}
				Timer->remark_id=remarkId_set;
				Timer->tickcount=0;
			}
			return door_operator[index].func(client);
		}
	}
	if(type_is_exist==0){
		return send_to_client(client,"error");
	}
	cJSON_Delete(root);

badJson:
	if(root!=NULL){
		cJSON_Delete(root);
	}
	if(client_sign==CLIENT_IS_HTTP){
		http_response_BAD_REQUEST(client);
		return HTTPD_CGI_DONE;
	}
	if(client_sign==CLIENT_IS_MQTT){
		send_to_client(client,"error");
		os_free(data);
	}
	return 1;
}


int ICACHE_FLASH_ATTR send_to_client(void *client,char *message){
	if(*(int *)client==CLIENT_IS_MQTT){
		MQTT_Client *mqtt_client=(MQTT_Client *)client;
		mqtt_door_comm_data *mqtt_user_data=(mqtt_door_comm_data *)(mqtt_client->user_data);
		MQTT_Publish(client,mqtt_user_data->pub_topic,message,os_strlen(message),mqtt_user_data->pub_qos,0);
		if(mqtt_client->user_data!=NULL)
			os_free(((MQTT_Client *)client)->user_data);
	}else if(*(int *)client==CLIENT_IS_HTTP){
		http_write(client,message);
		http_connection *http_client=(http_connection *)client;
		if(http_client->cgi.data!=NULL)
			os_free(http_client->cgi.data);
		return HTTPD_CGI_DONE;
	}
	return 1;
}
int ICACHE_FLASH_ATTR door_wifi_config_read(void *client){
	cJSON *root,*data;
	char *ostream;
	struct softap_config ap_config;
	wifi_softap_get_config(&ap_config);
	struct station_config sta_config;
	wifi_station_get_config(&sta_config);

	char temp[2];
	//0x01 Station 0x02 Ap
	uint8 work_mode=wifi_get_opmode();
	work_mode=(work_mode==0x03?0x02:work_mode)-1;
	temp[0]=work_mode+0x30;
	temp[1]=0;
	root = cJSON_CreateObject();
	data = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "type", "Reply_GetWiFiConfig");
	cJSON_AddItemToObject(root, "data", data);

	cJSON_AddStringToObject(data,"work_mode",temp);
	cJSON_AddStringToObject(data,"wifi_ap_ssid",ap_config.ssid);
	cJSON_AddStringToObject(data,"wifi_ap_pwd",ap_config.password);
	cJSON_AddStringToObject(data,"wifi_station_ssid",sta_config.ssid);
	cJSON_AddStringToObject(data,"wifi_station_pwd",sta_config.password);
	cJSON_AddStringToObject(root, "error_code","0" );
	cJSON_AddStringToObject(root, "error_str","" );

	ostream=cJSON_Print(root);
	//delete json struct
	uint8 ret=send_to_client(client,ostream);
	cJSON_Delete(root);
	os_free(ostream);
	return ret;
}
int ICACHE_FLASH_ATTR door_safe_config_read(void *client){
	NODE_DBG("http_safe_config_api_read");
	if(safe_config_refresh_get()==CONFIG_REFRESHED)
	{
		return send_to_client(client,safe_config_read());
	}
	return send_to_client(client,"error");
}
int ICACHE_FLASH_ATTR door_door_config_read(void *client){
	NODE_DBG("http_door_config_api_read");
	if(door_config_refresh_get()==CONFIG_REFRESHED)
	{
		return send_to_client(client,door_config_read());
	}
	return send_to_client(client,"error");
}
int ICACHE_FLASH_ATTR door_wifi_config_write(void *client){

}
int ICACHE_FLASH_ATTR door_door_expect_ret(void *client){

	client_handle_timer *Timer;
	if(*(int *)client==CLIENT_IS_HTTP){
		http_connection *http_client=(http_connection *)client;
		Timer=(client_handle_timer *)http_client->cgi.data;
		if(Timer->tickcount==0){
			os_memset(&Timer->timer,0,sizeof(os_timer_t));
			os_timer_disarm(&Timer->timer);
			os_timer_setfn(&Timer->timer, http_execute_cgi, client);
			os_timer_arm(&Timer->timer, TIMER_SINGLETIME, 1);
			//set header
			http_SET_HEADER(client,HTTP_CONTENT_TYPE,JSON_CONTENT_TYPE);
			http_response_OK(client);
		}
	}else if(*(int *)client==CLIENT_IS_MQTT){
		MQTT_Client *mqtt_client=(MQTT_Client *)client;
		mqtt_door_comm_data *mqtt_user_data=(mqtt_door_comm_data *)(mqtt_client->user_data);
		Timer=&mqtt_user_data->timer;
		if(Timer->tickcount==0){
			os_memset(&Timer->timer,0,sizeof(os_timer_t));
			os_timer_disarm(&Timer->timer);
			os_timer_setfn(&Timer->timer, door_door_expect_ret, client);
			os_timer_arm(&Timer->timer, TIMER_SINGLETIME, 1);
		}
	}
	Timer->tickcount++;
	if(Timer->tickcount<TIMER_TIMEROUT){
		if(door_others_remarkid_isExist(Timer->remark_id)){
			os_timer_disarm(&Timer->timer);
			return send_to_client(client,door_others_read(Timer->remark_id));
		}
	}else{
		os_timer_disarm(&Timer->timer);
		return send_to_client(client,"error");
	}

	if(*(int *)client==CLIENT_IS_HTTP){
		return HTTPD_CGI_MORE;
	}
	return 1;
}



void request_master_safe_config(){
	char resData[]="{\"type\":\"GetSafeConfig\",\"data\":\"\"}";
	safe_config_refresh_set(CONFIG_REFRESHING);
	send_message_to_master(0,0x02,resData,os_strlen(resData));
}

void request_master_door_config(){
	char resData[]="{\"type\":\"GetDoorConfig\",\"data\":\"\"}";
	door_config_refresh_set(CONFIG_REFRESHING);
	send_message_to_master(0,0x02,resData,os_strlen(resData));
}

void ICACHE_FLASH_ATTR door_request_all_config(){
	if(safe_config_refresh_get()==CONFIG_NOREFRESH)
		safe_config_refresh_set(CONFIG_REFRESHING);
	else if(safe_config_refresh_get()==CONFIG_REFRESHING)
		request_master_safe_config();

	if(door_config_refresh_get()==CONFIG_NOREFRESH)
		door_config_refresh_set(CONFIG_REFRESHING);
	else  if(door_config_refresh_get()==CONFIG_REFRESHING)
		request_master_door_config();
}







