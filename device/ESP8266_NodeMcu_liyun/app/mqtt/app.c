#include "c_types.h"
#include "c_string.h"
#include "c_stdio.h"
#include "user_interface.h"
#include "mem.h"
#include "osapi.h"
#include "user_config.h"
#include "espconn.h"
#include "app.h"
#include "mqtt.h"
#include "mqtt_msg.h"

#include "aliyun_mqtt.h"

#include "driver/relay.h"
#include "comm/comm.h"

#include "sensor/sensors.h"

#include "json/cJson.h"

static os_timer_t mqtt_timer;

static MQTT_Client mqtt_client;

static uint8_t wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;

typedef struct mqtt_relay{
	uint8_t id;
	char topic[20];		
}mqtt_relay;

mqtt_relay relays[RELAY_COUNT];

static char *ali_sub_topics[]={
		SUB_TOPIC,SUB_RRPC_TOPIC,NULL
};

typedef void(*mqtt_topic_callback)(MQTT_Client *client);

typedef struct{
	const char *type;
	mqtt_topic_callback func;
	int flag;
}door_def;


static void ICACHE_FLASH_ATTR mqtt_app_timer_cb(void *arg){
	struct ip_info ipConfig;

		wifi_get_ip_info(STATION_IF, &ipConfig);
		wifiStatus = wifi_station_get_connect_status();
		//check wifi
		if(wifiStatus != lastWifiStatus){

			lastWifiStatus = wifiStatus;

			if(wifiStatus==STATION_GOT_IP && ipConfig.ip.addr != 0){
	        	MQTT_DBG("MQTT: Detected wifi network up,trying connect to :%s",MQTT_SERVER_IP);
	        	MQTT_Connect(&mqtt_client);
		    }
		    else{
		    	MQTT_DBG("MQTT: Detected wifi network down");
		    	MQTT_Disconnect(&mqtt_client);
		    }
		}
	}

static void mqttConnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	MQTT_DBG("MQTT: Connected");
	int i;
	for(i=0;ali_sub_topics[i]!=NULL;i++){
		MQTT_Subscribe(client, ali_sub_topics[i],0);
		os_printf("sub:%s\r\n",ali_sub_topics[i]);
	}
	char data[]="Wireless Connected.";
	MQTT_DBG("MQTT: Publish:%s,length:%d",data,os_strlen(data));
	//MQTT_Publish(client,RES_TOPIC,data,os_strlen(data),1,0);
}

static void mqttDisconnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	MQTT_DBG("MQTT: Disconnected");
}

static void mqttPublishedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	MQTT_DBG("MQTT: Published");
}

static void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
	char *topicBuf = (char*)os_zalloc(topic_len+1),
			*dataBuf = (char*)os_zalloc(data_len+1);

	MQTT_Client* client = (MQTT_Client*)args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	//MQTT_DBG("Receive topic: %s, data: %s ", topicBuf, dataBuf);
	os_printf("Receive topic: %s, data: %s\r\n", topicBuf, dataBuf);

	//undecision
	os_free(topicBuf);
	os_free(dataBuf);

	int i;
//	char *rrpc_topic_head="/sys/a16U9ZZ9jb5/NWOYa1LR2HNPV8hVyXeW/rrpc/request/";
	if(os_strncmp(topic,SUB_RRPC_TOPIC,os_strlen(SUB_RRPC_TOPIC)-1)==0){
		char msg_id[50];
		os_strncpy(msg_id,topic+os_strlen(SUB_RRPC_TOPIC)-1,topic_len-os_strlen(SUB_RRPC_TOPIC)+1);
		char response_tp[100]="/sys/a16U9ZZ9jb5/NWOYa1LR2HNPV8hVyXeW/rrpc/response/";
		os_strcat(response_tp,msg_id);
		mqtt_operator_api(response_tp,data,data_len);
		return;
	}
	for(i=0;ali_sub_topics[i]!=NULL;i++){
		if(os_strncmp(topic,ali_sub_topics[i],topic_len)==0){
			mqtt_operator_api(NULL,data,data_len);
			break;
		}
	}
}


int ICACHE_FLASH_ATTR mqtt_is_connected(){
	//enable reconnect
	switch(mqtt_client.connState){
	case WIFI_INIT:
	case WIFI_CONNECTING:
	case WIFI_CONNECTING_ERROR:
	case WIFI_CONNECTED:
	case DNS_RESOLVE:
	case TCP_DISCONNECTED:
	case TCP_RECONNECT_REQ:
	case TCP_RECONNECT:
	case TCP_CONNECTING:
	case TCP_CONNECTING_ERROR:
		return 0;
	case TCP_CONNECTED:
	case MQTT_CONNECT_SEND:
	case MQTT_CONNECT_SENDING:
	case MQTT_SUBSCIBE_SEND:
	case MQTT_SUBSCIBE_SENDING:
	case MQTT_DATA:
	case MQTT_PUBLISH_RECV:
	case MQTT_PUBLISHING:
		return 1;
	}
}


void ICACHE_FLASH_ATTR mqtt_publish_api(const char* topic, const char* data, int data_length, int qos, int retain){

	MQTT_DBG("MQTT: client is:%d",&mqtt_client);
	MQTT_Publish(&mqtt_client,topic,data,data_length,qos,retain);
}
void ICACHE_FLASH_ATTR mqtt_app_init(){
	aliyun_mqtt_init();
	MQTT_InitConnection(&mqtt_client, cst_al_mqtt.host, cst_al_mqtt.port, 0);
	MQTT_InitClient(&mqtt_client, cst_al_mqtt.client_id, cst_al_mqtt.username,cst_al_mqtt.password, cst_al_mqtt.keepalive, 1);


	//MQTT_InitConnection(&mqtt_client, MQTT_SERVER_IP, MQTT_SERVER_PORT, 0);
	//MQTT_InitClient(&mqtt_client, "ThisIsClientId", "", "", 120, 1);
	MQTT_OnConnected(&mqtt_client, mqttConnectedCb);
	MQTT_OnDisconnected(&mqtt_client, mqttDisconnectedCb);
	MQTT_OnPublished(&mqtt_client, mqttPublishedCb);
	MQTT_OnData(&mqtt_client, mqttDataCb);
	MQTT_DBG("MQTT: timer,try connect");

	//arm mqtt timer
	os_memset(&mqtt_timer,0,sizeof(os_timer_t));
	os_timer_disarm(&mqtt_timer);
	os_timer_setfn(&mqtt_timer, (os_timer_func_t *)mqtt_app_timer_cb, NULL);
	os_timer_arm(&mqtt_timer, 1000, 1);

}
