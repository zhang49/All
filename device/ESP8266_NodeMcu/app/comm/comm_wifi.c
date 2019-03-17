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
#include "espconn.h"
#include "c_stdio.h"
#include "user_config.h"
#include <limits.h>

#include "comm.h"
#include "comm/comm_wifi.h"

#include "json/cJson.h"

//WiFi access point data
typedef struct {
	char ssid[32];
	char rssi;
	char enc;
	char channel;
} ap;

//Scan result
typedef struct {	
	ap **ap;
	int ap_count;
} scan_result_data;

enum OperatorStatus{
	Start,
	Doing,
	Done,
	Extra
}operator_status;

typedef struct { 
	enum OperatorStatus scanState;
	uint8_t connecting;
	scan_result_data scan_result;
	uint8_t mode;
	uint8_t station_status;
	struct station_config station_config;

} wifi_status_t;


typedef struct {
	uint8_t state;
	ETSTimer timer;
} api_cgi_status;

typedef struct {
	uint8_t state;
	ETSTimer timer;
	char ssid[32];
	char pwd[64];
} api_cgi_connect_status;

typedef struct {
	uint8_t state;
	http_connection *http_client;	
} api_cgi_check_internet_status;

typedef struct {
	uint8_t state;
	ETSTimer timer;
	int ap_index;
} api_cgi_scan_status;

static wifi_status_t wifi_status;

struct station_config *wifi_st_config;


/*
 *WiFi scan callback function
 */
static void ICACHE_FLASH_ATTR comm_wifi_api_scan_callback(void *arg, STATUS status){

	int n;
	struct bss_info *bss_link = (struct bss_info *)arg;
	NODE_DBG("Wifi Scan Done, status: %d", status);
	if (status!=OK) {
		wifi_status.scanState=Doing;
		return;
	}

	//Clear prev ap data if needed.
	if (wifi_status.scan_result.ap!=NULL) {
		for (n=0; n<wifi_status.scan_result.ap_count; n++) 
			os_free(wifi_status.scan_result.ap[n]);
		os_free(wifi_status.scan_result.ap);
	}

	//Count amount of access points found.
	n=0;
	while (bss_link != NULL) {
		bss_link = bss_link->next.stqe_next;
		n++;
	}

	//Allocate memory for access point data
	wifi_status.scan_result.ap=(ap **)os_malloc(sizeof(ap *)*n);
	wifi_status.scan_result.ap_count=n;
	NODE_DBG("Scan done: found %d APs", n);

	//Copy access point data to the static struct
	n=0;
	bss_link = (struct bss_info *)arg;
	while (bss_link != NULL) {
		if (n>=wifi_status.scan_result.ap_count) {
			//This means the bss_link changed under our nose. Shouldn't happen!
			//Break because otherwise we will write in unallocated memory.
			NODE_DBG("Huh? I have more than the allocated %d aps!", wifi_status.scan_result.ap_count);
			break;
		}
		//Save the ap data.
		if(strlen(bss_link->ssid)>0){
			wifi_status.scan_result.ap[n]=(ap *)os_malloc(sizeof(ap));
			wifi_status.scan_result.ap[n]->rssi=bss_link->rssi;
			wifi_status.scan_result.ap[n]->enc=bss_link->authmode;
			wifi_status.scan_result.ap[n]->channel=bss_link->channel;
			strncpy(wifi_status.scan_result.ap[n]->ssid, (char*)bss_link->ssid, 32);			
			n++;
		}
		else{
			wifi_status.scan_result.ap_count--;
		}

		bss_link = bss_link->next.stqe_next;
		
	}

	//We're done.
	wifi_status.scanState=Done;

}

/*
 *WiFi start scaning
 */
void ICACHE_FLASH_ATTR comm_wifi_api_scan_start(){
	NODE_DBG("http_wifi_api_scan");
	if(wifi_status.scanState!=Doing){
		wifi_status.scanState=Start;
	}
}
/*
 *WiFi scaning return NULL
 *WiFi scan over return jsonStr
 */


char *ICACHE_FLASH_ATTR comm_wifi_api_scan() {
	 switch(wifi_status.scanState){
	 case Start:
		 NODE_DBG("Start scan");
		 wifi_station_scan(NULL,comm_wifi_api_scan_callback);
		 wifi_status.scanState=Doing;
		 break;
	 case Doing:
		 NODE_DBG("Waiting scan done");
		 break;
	 case Done:
		operator_status=Extra;
		//create json

		cJSON *data,*array,*item;
		cJSON *retroot=create_ret_json("Reply_GetWiFiScan");
		cJSON_AddItemToObject(retroot, "data", data = cJSON_CreateObject());
		cJSON_AddNumberToObject(data,"ap_count",wifi_status.scan_result.ap_count);
		cJSON_AddItemToObject(data, "ap", array = cJSON_CreateArray());
		cJSON_AddNumberToObject(retroot,"error_code",0);
		cJSON_AddStringToObject(retroot,"error_str","");
		/*
		//check max count on query string
		char *query=http_url_get_query_param(c,"max");
		int max = INT_MAX;
		if(query!=NULL)
			max = atoi(query);
		*/
		//data too long will send failed
		int max = 10;
		int i,j,index;
		for(i=0;i< wifi_status.scan_result.ap_count && i<max;i++){
			uint8 rssi=1;
			for(j=0;j< wifi_status.scan_result.ap_count;j++){
				if(wifi_status.scan_result.ap[j]->rssi>rssi){
					index=j;
				}
			}
			cJSON_AddItemToObject(array,"",item=cJSON_CreateObject());
			cJSON_AddStringToObject(item,"ssid",(const char *)wifi_status.scan_result.ap[index]->ssid);
			cJSON_AddNumberToObject(item,"rssi",wifi_status.scan_result.ap[index]->rssi);
			cJSON_AddNumberToObject(item,"enc",wifi_status.scan_result.ap[index]->enc);
			cJSON_AddNumberToObject(item,"channel",wifi_status.scan_result.ap[index]->channel);
			wifi_status.scan_result.ap[index]->rssi=0;
		}
		char *ostream=cJSON_Print(retroot);
		cJSON_Delete(retroot);
		return ostream;
		break;
	 case Extra:

		 break;
	 }
	 return NULL;


}
int ICACHE_FLASH_ATTR comm_wifi_safe_read(void *client){
	uint8 mac[6];
	wifi_get_macaddr(SOFTAP_IF,mac);
	cJSON *retroot,*data;
	retroot = create_ret_json("Reply_GetSafeConfig");
	data = cJSON_CreateObject();
	cJSON_AddItemToObject(retroot, "data", data);
	char mac_addr[25];
	char temp[3];
	int i;
	os_memset(mac_addr,0,sizeof(os_timer_t));
	for(i=0;i<6;i++){
		if(i!=0)os_strcat(mac_addr,"-");
		if(mac[i]<0x0f)
			os_sprintf(temp,"0%x",mac[i]);
		else
			os_sprintf(temp,"%x",mac[i]);
		os_strcat(mac_addr,temp);
	}
	cJSON_AddStringToObject(data,"mac",mac_addr);
	return send_ret_json(client,retroot,EC_Normal);
}
int ICACHE_FLASH_ATTR comm_wifi_config_read(void *client){
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
	cJSON *data;
	cJSON *retroot=create_ret_json("Reply_GetWiFiConfig");
	cJSON_AddItemToObject(retroot,"data",data = cJSON_CreateObject());
	cJSON_AddStringToObject(data,"work_mode",temp);
	cJSON_AddStringToObject(data,"wifi_ap_ssid",ap_config.ssid);
	cJSON_AddStringToObject(data,"wifi_ap_pwd",ap_config.password);
	cJSON_AddStringToObject(data,"wifi_station_ssid",sta_config.ssid);
	cJSON_AddStringToObject(data,"wifi_station_pwd",sta_config.password);
	struct ip_info ip;
	wifi_get_ip_info(0x0,&ip);
	char *ip_str = ipaddr_ntoa(&ip.ip);
	cJSON_AddStringToObject(data,"wifi_station_ip",ip_str);

	return send_ret_json(client,retroot,EC_Normal);
}

int ICACHE_FLASH_ATTR comm_wifi_config_write(void *client){
		char *data;
		uint8 client_sign=*(int *)client;
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
		//parse json
		cJSON *retroot=create_ret_json("Reply_SetWiFiConfig");
		enum ErrorCode error_code=EC_Normal;
		cJSON *root = cJSON_Parse(data);
		cJSON *root_data = cJSON_GetObjectItem(root,"data");
		if(root_data==NULL){
			error_code=EC_None;
			goto badJson;
		}
		int work_mode=cJSON_GetObjectItem(root_data,"work_mode")->valueint;
		//set station+ap mode for test
		work_mode=0x03;
	    wifi_set_opmode(work_mode);
		cJSON *json_ap_ssid=cJSON_GetObjectItem(root_data,"wifi_ap_ssid");
		/*if(json_ap_ssid!=NULL){
			struct softap_config config;
			wifi_softap_get_config(&config);
			strcpy(config.ssid,json_ap_ssid->valuestring);
			memset(config.password,0,64);
			strcpy(config.password,cJSON_GetObjectItem(root_data,"wifi_ap_pwd")->valuestring);
			config.ssid_len=strlen(json_ap_ssid->valuestring);
			config.channel=11;
			config.authmode=AUTH_OPEN;
			config.max_connection=5;
			config.ssid_hidden=0;
			wifi_softap_set_config(&config);
		}*/
		cJSON *sta_ssid=cJSON_GetObjectItem(root_data,"wifi_station_ssid");
		cJSON *sta_pwd=cJSON_GetObjectItem(root_data,"wifi_station_pwd");
		if(comm_wifi_connect_ap(sta_ssid->valuestring,sta_pwd->valuestring)==Done){
			error_code=EC_Normal;
		}
badJson:
	cJSON_Delete(root);
	if(client_sign==CLIENT_IS_MQTT){
		os_free(data);
	}
	return send_ret_json(client,retroot,error_code);

}
char last_sta_ssid[32];
char last_sta_pwd[64];

int ICACHE_FLASH_ATTR comm_wifi_connect_ap(char *ssid,char *password){
	//simple , need more operator
	if(wifi_status.connecting==Doing)return 0;
	wifi_status.connecting=Start;
	struct station_config sta_config;
	wifi_station_get_config(&sta_config);
	NODE_DBG("Putting STATION UP");
	uint8 status = wifi_station_get_connect_status();
	strcpy(wifi_status.station_config.ssid,sta_config.ssid);
	strcpy(wifi_status.station_config.password,sta_config.password);
	if(status!=STATION_CONNECTING && status!=STATION_GOT_IP)
	{
		wifi_status.connecting=1;
		wifi_station_disconnect();
		strcpy(sta_config.ssid,ssid);
		strcpy(sta_config.password,password);
		//don't write in flash;
		wifi_station_set_config_current(&sta_config);
		wifi_station_connect();
	}
	wifi_status.connecting=Done;
	return Done;
}

