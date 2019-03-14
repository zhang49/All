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

typedef struct { 
	uint8_t scanning;
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
		wifi_status.scanning=0;
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
	wifi_status.scanning=0;

}

/*
 *WiFi start scaning
 */
void ICACHE_FLASH_ATTR comm_wifi_api_scan_start(){
	NODE_DBG("http_wifi_api_scan");
	if(!wifi_status.scanning){
		//if not already scanning, request scan
		NODE_DBG("Starting scan");
		wifi_station_scan(NULL,comm_wifi_api_scan_callback);
		wifi_status.scanning=1;
	}
}
/*
 *WiFi scaning return NULL
 *WiFi scan over return jsonStr
 */
enum OperatorStatus{
	Start,
	Doing,
	Done,
	Extra
}operator_status;

char *ICACHE_FLASH_ATTR comm_wifi_api_scan() {
	 switch(operator_status){
	 case Start:
		 NODE_DBG("Start scan");
		 comm_wifi_api_scan_start();
		 operator_status=Doing;
		 break;
	 case Doing:
		 NODE_DBG("Waiting scan done");
		 break;
	 case Done:
		operator_status=Extra;
		//create json
		cJSON *root,*data,*array,*item;
		cJSON * item;
		root = cJSON_CreateObject();
		data = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"type","Reply_GetWiFiScan");
		cJSON_AddItemToObject(root, "data", data);
		cJSON_AddNumberToObject(data,"ap_count",wifi_status.scan_result.ap_count);
		cJSON_AddItemToObject(data, "ap", array = cJSON_CreateArray());
		/*
		//check max count on query string
		char *query=http_url_get_query_param(c,"max");
		int max = INT_MAX;
		if(query!=NULL)
			max = atoi(query);
		*/
		int max = 10;
		int i;
		for(i=0;i< wifi_status.scan_result.ap_count && i<max;i++){

			cJSON_AddItemToArray(array,item=cJSON_CreateObject());
			cJSON_AddStringToObject(item,"ssid",(const char *)wifi_status.scan_result.ap[i]->ssid);
			cJSON_AddNumberToObject(item,"rssi",wifi_status.scan_result.ap[i]->rssi);
			cJSON_AddNumberToObject(item,"enc",wifi_status.scan_result.ap[i]->enc);
			cJSON_AddNumberToObject(item,"channel",wifi_status.scan_result.ap[i]->channel);
		}
		char *ostream=cJSON_Print(root);
		cJSON_Delete(root);
		return ostream;
		break;
	 case Extra:

		 break;
	 }
	 return NULL;


}
int ICACHE_FLASH_ATTR comm_wifi_config_read(void *client){
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

	struct ip_info ip;
	wifi_get_ip_info(0x0,&ip);
	char *ip_str = ipaddr_ntoa(&ip.ip);
	cJSON_AddStringToObject(data,"wifi_station_ip",ip_str);
	cJSON_AddStringToObject(root, "error_code","0" );
	cJSON_AddStringToObject(root, "error_str","" );

	ostream=cJSON_Print(root);
	//delete json struct
	uint8 ret=send_to_client(client,ostream);
	cJSON_Delete(root);
	os_free(ostream);
	return ret;
}

int ICACHE_FLASH_ATTR comm_wifi_config_write(void *client){

}
char last_sta_ssid[32];
char last_sta_pwd[64];
void ICACHE_FLASH_ATTR comm_wifi_connect_ap(char *ssid,char *password){
	if(wifi_status.connecting==1)return;
	struct station_config sta_config;
	wifi_station_get_config(&sta_config);
	NODE_DBG("Putting STATION UP");
	uint8 status = wifi_station_get_connect_status();
	strcpy(wifi_status.station_config.ssid,sta_config.ssid);
	strcpy(wifi_status.station_config.password,sta_config.password);
	if(status!=STATION_CONNECTING && status!=STATION_GOT_IP)
	{
		wifi_status.connecting=1;
		strcpy(sta_config.ssid,ssid);
		strcpy(sta_config.password,password);
		wifi_station_set_config(&sta_config);
		wifi_station_connect();
	}
}

