/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Israel Lot <me@israellot.com> and Jeroen Domburg <jeroen@spritesmods.com> 
 * wrote this file. As long as you retain this notice you can do whatever you 
 * want with this stuff. If we meet some day, and you think this stuff is 
 * worth it, you can buy us a beer in return. 
 * ----------------------------------------------------------------------------
 */

#include "c_string.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "c_stdio.h"
#include "c_stdlib.h"
#include "platform.h"
#include "user_config.h"
//#include "driver/relay.h"
//#include "sensor/sensors.h"

#include "cgi.h"
#include "cgi_door.h"

#include "http.h"
#include "http_parser.h"
#include "http_server.h"
#include "http_process.h"
#include "http_helper.h"
#include "http_client.h"
#include "door/door.h"
#include "door/door_uart.h"

#include "json/cJson.h"

typedef struct{
	char *type;
	http_callback func;
	int flag;
}door_def;

typedef struct {
	uint8_t state;
	ETSTimer timer;
	uint8_t remark_id;
	uint8_t op;
	uint8_t tickcount;
} api_cgi_status;

static uint8_t remark_id=0;

//NEEDTIMER is a timer to check data is received
static door_def door_operator[]={
		{"GetWiFiConfig",	http_wifi_config_api_read,		NULL},
		{"GetSafeConfig",	http_safe_config_api_read,		NULL},
		{"GetDoorConfig",	http_door_config_api_read,		NULL},
		{"SetWiFiConfig",	http_wifi_config_api_write,		NULL},
		{"SetSafeConfig",	http_door_expect_ret,			NEEDTIMER},
		{"SetDoorConfig",	http_door_expect_ret,			NEEDTIMER},
		{"Command",			http_door_expect_ret,			NEEDTIMER},
		{NULL,				NULL,							NULL}
};

int ICACHE_FLASH_ATTR http_door_operator(http_connection *c)
{

	NODE_DBG("http_door_operator");
	//wait for whole body
	if(c->state <HTTPD_STATE_BODY_END)
		return HTTPD_CGI_MORE;
	//for test
	static int count=0;
	count++;
	if(c->body.data==NULL){
		uart_write_string(0,"c->body.data is NULL\r\n");
	}
	//parse json
	cJSON *root = cJSON_Parse(c->body.data);
	if(root==NULL){
		uart_write_string(0,"http_door_command_api_read cJSON_Parse error\r\n");
		goto badrequest;
	}

	cJSON *type = cJSON_GetObjectItem(root,"type");
	if(type==NULL){
		uart_write_string(0,"http_door_command_api_read cJSON_GetObjectItem error\r\n");
		goto badrequest;
	}

	NODE_DBG("request type :%s",type->valuestring);
	int index=0;
	while(door_operator[index].type!=NULL){
		if(os_strcmp(type->valuestring,door_operator[index].type)==0){
			if(door_operator[index].flag==NULL){
				http_SET_HEADER(c,HTTP_CONTENT_TYPE,JSON_CONTENT_TYPE);
				http_response_OK(c);
				if(!door_operator[index].func(c))goto badrequest;
				cJSON_Delete(root);
				return HTTPD_CGI_DONE;
			}else if(door_operator[index].flag==NEEDTIMER){
				cJSON_Delete(root);
				do
				{
					api_cgi_status *status = (api_cgi_status*)os_malloc(sizeof(api_cgi_status));
					status->state=1;
					status->tickcount=0;
					status->remark_id=0;
					status->op=index;
					c->cgi.data=status;
					//set timer to check again
					os_memset(&status->timer,0,sizeof(os_timer_t));
					os_timer_disarm(&status->timer);
					os_timer_setfn(&status->timer, http_execute_cgi, c);
					os_timer_arm(&status->timer, TIMER_SINGLETIME, 1);
				}while(0);
				c->cgi.function=door_operator[index].func;
				return door_operator[index].func(c);
			}
		}
		index++;
	}

badrequest:
	http_response_BAD_REQUEST(c);
	cJSON_Delete(root);
	uart_write_string(0,"Bad request\r\n");
	return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR http_wifi_config_api_write(http_connection *c)
{
	cJSON *root = cJSON_Parse(c->body.data);
	cJSON *data = cJSON_GetObjectItem(root,"data");
	if(data==NULL){
		NODE_DBG("http_door_command_api_read cJSON_GetObjectItem error");
		return 0;
	}
	cJSON *ap_ssid = cJSON_GetObjectItem(data,"wifi_ap_ssid");
	cJSON *ap_pwd = cJSON_GetObjectItem(data,"wifi_ap_pwd");
	cJSON *station_ssid = cJSON_GetObjectItem(data,"wifi_station_ssid");
	cJSON *station_pwd = cJSON_GetObjectItem(data,"wifi_station_pwd");
	cJSON *wm = cJSON_GetObjectItem(data,"work_mode");
	struct softap_config ap_config;
	struct station_config sta_config;
	wifi_softap_get_config(&ap_config);
	wifi_station_get_config(&sta_config);
	os_strcpy(ap_config.ssid,ap_ssid->valuestring);
	os_strcpy(ap_config.password,ap_pwd->valuestring);
	os_strcpy(ap_config.ssid,ap_ssid->valuestring);
	os_strcpy(ap_config.password,ap_pwd->valuestring);
	wifi_softap_set_config(&ap_config);
	wifi_station_set_config(&sta_config);
	uint8 work_mode=c_atoi(wm->valuestring);
	work_mode+=1;
	uint8 work_mode_cur=wifi_get_opmode();
	if(work_mode!=work_mode_cur)
	{
		wifi_set_opmode(work_mode);
	}
	cJSON_Delete(root);
	return HTTPD_CGI_DONE;
}
int ICACHE_FLASH_ATTR http_wifi_config_api_read(http_connection *c)
{
	NODE_DBG("http_door_config_api_read");

	cJSON *root, *data, *fld;

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

	cJSON_AddStringToObject(root, "type", "Reply_GetWiFiConfig");
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());

	cJSON_AddStringToObject(data,"work_mode",temp);
	cJSON_AddStringToObject(data,"wifi_ap_ssid",ap_config.ssid);
	cJSON_AddStringToObject(data,"wifi_ap_pwd",ap_config.password);
	cJSON_AddStringToObject(data,"wifi_station_ssid",sta_config.ssid);
	cJSON_AddStringToObject(data,"wifi_station_pwd",sta_config.password);
	cJSON_AddStringToObject(root, "error_code","0" );
	cJSON_AddStringToObject(root, "error_str","" );
	//
	http_write_json(c,root);

	//delete json struct
	cJSON_Delete(root);

	return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR http_door_config_api_read(http_connection *c)
{
	NODE_DBG("http_door_config_api_read");
	if(door_config_isrefresh())
	{
		NODE_DBG("http_write door_config");
		http_write(c,get_door_config());
	}else{
		//send data to master get config
	}

	return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR http_door_expect_ret(http_connection *c)
{
	//send Header Message, touch send_cb
	api_cgi_status * status = c->cgi.data;
	if(status->remark_id==0){
		remark_id++;
		status->remark_id=remark_id;
		send_message_to_master(status->remark_id,0x02,c->body.data,os_strlen(c->body.data));
		return HTTPD_CGI_MORE;
	}else{
		status->tickcount++;
		if(door_databuf_remarkid_isExist(status->remark_id)){
			status->state=99;
			http_SET_HEADER(c,HTTP_CONTENT_TYPE,JSON_CONTENT_TYPE);
			http_response_OK(c);
			http_write(c,door_others_read(status->remark_id));
		}else if(status->tickcount>TIMER_TIMEROUT){
			status->state=99;
			http_SET_HEADER(c,HTTP_CONTENT_TYPE,JSON_CONTENT_TYPE);
			http_response_OK(c);
			http_write(c,"error");
		}
	}
	if(status->state==99){
		os_timer_disarm(&status->timer);
		if(os_strcmp(door_operator[status->op].type,"SetDoorConfig")==0){
			door_config_refresh_set(0);
		}		os_free(c->cgi.data);
		return HTTPD_CGI_DONE;
	}
	return HTTPD_CGI_MORE;
}


int ICACHE_FLASH_ATTR http_safe_config_api_read(http_connection *c)
{

	NODE_DBG("http_safe_config_api_read");
	if(safe_config_isrefresh())
	{
		NODE_DBG("http_write safe_config");
		http_write(c,get_safe_config());
	}

	/*cJSON *root, *data, *fld;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "type", "safe_test");
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());
	cJSON_AddStringToObject(data,"safe_config","None");

	http_write_json(c,root);

	//delete json struct
	cJSON_Delete(root);
    */

	return HTTPD_CGI_DONE;
}


