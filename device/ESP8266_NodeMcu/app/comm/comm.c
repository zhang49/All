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

#include "comm/comm_uart.h"
#include "comm/comm_wifi.h"
#include "http/app.h"

typedef int (*comm_operator_func)(void *);

typedef struct{
	const char *type;
	enum MasterMsgType msg_type;
	comm_operator_func func;
	int flag;
}door_def;


static door_def comm_operator[]={
		{"GetWiFiConfig",		NULL,				comm_wifi_config_read,		NULL},
		{"SetWiFiConfig",		NULL,				comm_wifi_config_write,		NULL},
		{"GetWiFiScan",			NORMAL_WIFI_SCAN,	comm_expect_ret,			NEEDTIMER},
		{"Control",				SYN_CONTROL,		comm_expect_ret,			NEEDTIMER},
		{"GetStatus",			SYN_STATE,			door_syn_status_read,		NULL},
		{NULL,					NULL,				NULL,						NULL},

};
//extern door_comm_buf comm_buf;

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

int ICACHE_FLASH_ATTR common_operator_api(void *client){
	char *data;
	uint8 client_sign=*(int *)client;
	http_connection *http_client;
	mqtt_comm_data *mqtt_user_data;
	if(client_sign==CLIENT_IS_MQTT){
		MQTT_Client *mqtt_client=(MQTT_Client *)client;
		//malloc??????
		mqtt_user_data=(mqtt_comm_data *)mqtt_client->user_data;
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
					os_free(data);
					Timer=&mqtt_user_data->timer;
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
							uint8 c_type=cJSON_GetObjectItem(c_data,"CommandType")->valueint;
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
							syn_control_refresh_set(COMM_NOREFRESH);
							send_message_to_master(SYN_CONTROL,&typevaule,1);
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
		mqtt_comm_data *mqtt_user_data=(mqtt_comm_data *)(mqtt_client->user_data);
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

int ICACHE_FLASH_ATTR door_door_config_read(void *client){
	NODE_DBG("http_door_config_api_read");
	if(door_config_refresh_get()==COMM_REFRESHED)
	{
		return send_to_client(client,door_config_read());
	}
	return send_to_client(client,"error");
}

int ICACHE_FLASH_ATTR door_syn_status_read(void *client){
	int ret=HTTPD_CGI_DONE;
	cJSON *retroot,*data;
	retroot=cJSON_CreateObject();
	cJSON_AddStringToObject(retroot,"type","Reply_GetStatus");
	cJSON_AddItemToObject(retroot,"data",data=cJSON_CreateObject());
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.sm_state),syn_state.sm_state);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.comm_state),syn_state.comm_state);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.temperature),syn_state.temperature);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.wetness),syn_state.wetness);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.power),syn_state.power);
	cJSON_AddNumberToObject(data,GET_LAST_STR(syn_state.run_time),syn_state.run_time);
	char *ostream=cJSON_Print(retroot);
	ret=send_to_client(client,ostream);
	cJSON_Delete(retroot);
	os_free(ostream);
	return ret;
}


int ICACHE_FLASH_ATTR comm_expect_ret(void *client){
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
		mqtt_comm_data *mqtt_user_data=(mqtt_comm_data *)(mqtt_client->user_data);
		Timer=&mqtt_user_data->timer;
		if(Timer->tickcount==0){
			os_memset(&Timer->timer,0,sizeof(os_timer_t));
			os_timer_disarm(&Timer->timer);
			os_timer_setfn(&Timer->timer, comm_expect_ret, client);
			os_timer_arm(&Timer->timer, TIMER_SINGLETIME, 1);
		}

	}
	Timer->tickcount++;
	//recv refresh data or timeout
	switch(Timer->msgtype){
	case NORMAL_CONFIG:
		if(door_config_refresh_get()==COMM_REFRESHED || Timer->tickcount>TIMER_TIMEROUT){
			os_timer_disarm(&Timer->timer);
			//modify config, must request config
			door_config_refresh_set(COMM_NOREFRESH);
			return send_to_client(client,door_config_read());
		}
		break;
	case SYN_CONTROL:
		if(syn_control_refresh_get()==COMM_REFRESHED || Timer->tickcount>TIMER_TIMEROUT){
			os_timer_disarm(&Timer->timer);
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
			}else if(Timer->tickcount>TIMER_TIMEROUT*10){
				os_timer_disarm(&Timer->timer);
				send_to_client(client,"error");
			}
		}
		break;
	}
	if(*(int *)client==CLIENT_IS_HTTP){
		return HTTPD_CGI_MORE;
	}
	return 1;
}

void request_master_normal_config(){
	door_config_refresh_set(COMM_REFRESHING);
	send_message_to_master(NORMAL_CONFIG,0x00,1);
}


void ICACHE_FLASH_ATTR request_master_door_config(){

	send_message_to_master(NORMAL_CONFIG,"",0);
}
void ICACHE_FLASH_ATTR door_request_all_config(){

	if(door_config_refresh_get()==COMM_NOREFRESH)
		door_config_refresh_set(COMM_REFRESHING);
	else  if(door_config_refresh_get()==COMM_REFRESHING)
		request_master_door_config();
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





