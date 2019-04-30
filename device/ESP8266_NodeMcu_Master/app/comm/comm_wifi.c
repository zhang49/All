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

#include "mqtt/app.h"
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
	enum OperatorStatus connectState;
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

/*
typedef struct {
	uint8_t state;
	http_connection *http_client;
} api_cgi_check_internet_status;
*/

typedef struct {
	uint8_t state;
	ETSTimer timer;
	int ap_index;
} api_cgi_scan_status;

static wifi_status_t wifi_status;

struct station_config *wifi_st_config;


static int wifi_reconnect_sign = 0;
static int wifi_tick_time_count = 0;
const int wifi_wait_time_max=5*60*1000;

int comm_positive = 0;

static uint8_t wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;

void wifi_connect_check(int tick_time){
	if(wifi_get_opmode() == SOFTAP_MODE){
		return;
	}
	uint8_t wifiStatus = wifi_station_get_connect_status();
	//check wifi
	struct ip_info ipConfig;
	wifi_get_ip_info(STATION_IF, &ipConfig);

	if(wifiStatus==STATION_GOT_IP && ipConfig.ip.addr != 0){
		wifi_reconnect_sign = 0;
		wifi_tick_time_count = 0;
		return;
	}else {
		/*  STATION_IDLE = 0,
		    STATION_CONNECTING,
			STATION_WRONG_PASSWORD,
			STATION_NO_AP_FOUND,
			STATION_CONNECT_FAIL,
		 */
		wifi_tick_time_count++;
		if(wifi_tick_time_count >= wifi_wait_time_max/tick_time){
			if(wifi_reconnect_sign==0){
				os_printf("reconnect count time out.\r\n");
				wifi_reconnect_sign=1;
				wifi_station_disconnect();
			}else if(wifi_reconnect_sign==5 && comm_positive==0){//has do.try to connect
				wifi_tick_time_count = 0;
			}
		}
		if( wifi_tick_time_count == 8000/tick_time || wifi_tick_time_count == 16000/tick_time){
			if(wifi_reconnect_sign==0){
				wifi_reconnect_sign=1;
				wifi_station_disconnect();
				os_printf("reconnect count < 2.\r\n");
			}else if(wifi_reconnect_sign==5 && comm_positive==0){//has do. try to connect
				wifi_tick_time_count++;
				//none
			}else{	//is busy,doesn't has try to connect
				wifi_tick_time_count--;
			}
		}else if(wifi_tick_time_count == 24000/tick_time){
			wifi_station_disconnect();
			os_printf("wifi_station_disconnect.\r\n");
		}
		//connect to station when communication is free
		switch(wifi_reconnect_sign){
		case 1:
			comm_positive=0;
		case 2:
		case 3:
		case 4:
			wifi_reconnect_sign++;
			break;
		case 5:
			if(comm_positive==0){
				os_printf("comm_positive==0 try to connect.\r\n");
				//connect to station
				wifi_reconnect_sign=0;
				wifi_get_ip_info(STATION_IF, &ipConfig);
				if(wifiStatus==STATION_GOT_IP && ipConfig.ip.addr != 0)return;
				struct station_config config;
				wifi_station_get_config_default(&config);
				wifi_station_set_config_current(&config);
				wifi_station_connect();
			} else {
				wifi_reconnect_sign=1;
			}
		}
	}
}
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
int ICACHE_FLASH_ATTR comm_wifi_scan_start_api(){
	NODE_DBG("door_wifi_scan_start");
	if(wifi_status.scanState!=Doing){
		wifi_status.scanState=Start;
		return 1;
	}
	return 0;
}
/*
 *WiFi scaning return NULL
 *WiFi scan over return jsonStr
 */


cJSON *ICACHE_FLASH_ATTR comm_wifi_scan_api(){
	 switch(wifi_status.scanState){
	 case Start:
		 wifi_status.scanState=Doing;
		 NODE_DBG("Start scan");
		 wifi_station_scan(NULL,comm_wifi_api_scan_callback);
		 break;
	 case Doing:
		 NODE_DBG("Waiting scan done");
		 break;
	 case Done:
		operator_status=Extra;
		//create json
		cJSON *data,*array,*item;
		cJSON *retroot=cJSON_CreateObject();
		cJSON_AddItemToObject(retroot, "data", data = cJSON_CreateObject());
		cJSON_AddNumberToObject(data,"ap_count",wifi_status.scan_result.ap_count);
		cJSON_AddItemToObject(data, "ap", array = cJSON_CreateArray());
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
		return retroot;
		break;
	 case Extra:

		 break;
	 }
	 return NULL;


}


cJSON * ICACHE_FLASH_ATTR comm_wifi_config_read_api(){
	uint8 mac[6];
	int i;
	char temp[3],mac_addr[50];
	struct softap_config ap_config;
	wifi_softap_get_config(&ap_config);
	struct station_config sta_config;
	wifi_station_get_config(&sta_config);
	wifi_get_macaddr(SOFTAP_IF,mac);


	cJSON *data;
	cJSON *retroot=cJSON_CreateObject();
	cJSON_AddItemToObject(retroot,"data",data = cJSON_CreateObject());

	os_memset(mac_addr,0,sizeof(mac_addr));
	for(i=0;i<6;i++){
		if(i!=0)os_strcat(mac_addr,"-");
		os_sprintf(temp,(mac[i]<0x0f?"0%x":"%x"),mac[i]);
		os_strcat(mac_addr,temp);
	}
	cJSON_AddStringToObject(data,"mac",mac_addr);
	//0x01 Station 0x02 Ap
	uint8 work_mode=wifi_get_opmode();
	work_mode=(work_mode==0x03?0x02:work_mode)-1;
	cJSON_AddNumberToObject(data,"work_mode",work_mode);
	cJSON_AddStringToObject(data,"wifi_ap_ssid",ap_config.ssid);
	cJSON_AddStringToObject(data,"wifi_ap_pwd",ap_config.password);
	cJSON_AddStringToObject(data,"wifi_station_ssid",sta_config.ssid);
	cJSON_AddStringToObject(data,"wifi_station_pwd",sta_config.password);
	struct ip_info ip;
	wifi_get_ip_info(0x0,&ip);
	char *ip_str = ipaddr_ntoa(&ip.ip);
	cJSON_AddStringToObject(data,"wifi_station_ip",ip_str);
	return retroot;
}

int ICACHE_FLASH_ATTR comm_wifi_config_write_api(cJSON *root_data){
	int work_mode=cJSON_GetObjectItem(root_data,"work_mode")->valueint;
	//set station+ap mode for test
	work_mode=0x03;
	wifi_set_opmode(work_mode);
	cJSON *json_ap_ssid=cJSON_GetObjectItem(root_data,"wifi_ap_ssid");
	cJSON *json_sta_ssid=cJSON_GetObjectItem(root_data,"wifi_station_ssid");
	if(json_sta_ssid!=NULL){
		/*
			struct station_config sta_config;
			os_strcpy(sta_config.bssid,json_ap_ssid->valuestring);
			os_strcpy(sta_config.password,cJSON_GetObjectItem(root_data,"wifi_station_pwd")->valuestring);
			wifi_station_set_config(&sta_config);
		*/
	}
	if(json_ap_ssid!=NULL){
		struct softap_config config;
		wifi_softap_get_config(&config);
		os_strcpy(config.ssid,json_ap_ssid->valuestring);
		os_memset(config.password,0,64);
		os_strcpy(config.password,cJSON_GetObjectItem(root_data,"wifi_ap_pwd")->valuestring);
		config.ssid_len=strlen(json_ap_ssid->valuestring);
		config.channel=11;
		config.authmode=AUTH_OPEN;
		config.max_connection=5;
		config.ssid_hidden=0;
		wifi_softap_set_config(&config);
	}
	return 1;
}

int ICACHE_FLASH_ATTR comm_wifi_start_connect_ap_api(char *ssid,char *password){
	if(wifi_status.connectState == Doing)return 0;
	wifi_status.connectState = Start;
	struct station_config sta_config;
	wifi_station_get_config(&sta_config);
	NODE_DBG("Station start try connect to: %s",ssid);
	//set current station config
	strcpy(sta_config.ssid,ssid);
	strcpy(sta_config.password,password);
	wifi_station_disconnect();
	wifi_station_set_config_current(&sta_config);
	wifi_station_connect();
	wifi_status.connectState = Doing;
	return 1;
}

int ICACHE_FLASH_ATTR comm_wifi_connect_ap_check_api(){
	//simple , need more operator
	if(wifi_status.connectState == Doing){
		wifi_tick_time_count = 0;
		if(mqtt_is_connected()){
			NODE_DBG("Station connect success");
			//保存到 Flash
			struct	station_config config;
			wifi_station_get_config(&config);
			wifi_station_set_config(&config);
			wifi_status.connectState=Done;
			return 1;
		}
	}
	return 0;
}

void ICACHE_FLASH_ATTR comm_wifi_connect_default_ap_api(){
	NODE_DBG("Station connect failed, connect default ap");
	struct	station_config config;
	wifi_station_disconnect();
	//get station config in flash
	wifi_station_get_config_default(&config);
	wifi_station_set_config_current(&config);
	wifi_station_connect();
	wifi_status.connectState = Done;
}


