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

//#include "aliyun_mqtt.h"

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

typedef struct{
	char *sub_topic;
	uint8 sub_qos;
	char *pub_topic;
	uint8 pub_qos;
}mqtt_topic_pair;

//pub topic:	"/"PRODUCT_KEY"/"DEVICE_NAME"/user/wireless/upload"
///a16U9ZZ9jb5/${deviceName}/user/app/request
//sub topic:	"/"PRODUCT_KEY"/"DEVICE_NAME"/user/app/request"

#define mqtt_upload_topic "/"MQTT_DEVICE_NAME"/user/wireless/upload"

static mqtt_topic_pair topics_pair[]={
		{"/"MQTT_DEVICE_NAME"/user/app/request",		0, 	"/"MQTT_DEVICE_NAME"/user/wireless/upload",	0},
		{NULL,											0,	NULL,										0}
};

typedef void(*mqtt_topic_callback)(MQTT_Client *client);

typedef struct{
	const char *type;
	mqtt_topic_callback func;
	int flag;
}door_def;


static int wifi_reconnect_count = 0;

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
		    	wifi_reconnect_count = 0;
		    }
		    else{
		    	MQTT_DBG("MQTT: Detected wifi network down");
		    	MQTT_Disconnect(&mqtt_client);
		    	wifi_reconnect_count++;
		    }
		}
		if(wifi_reconnect_count == 3){
			wifi_station_disconnect();
		}else if(wifi_reconnect_count == 100){
			wifi_reconnect_count = 0;
			struct station_config config;
			wifi_station_get_config_default(&config);
			wifi_station_set_config_current(&config);
			wifi_station_connect();
		}
	}

static void mqttConnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	MQTT_DBG("MQTT: Connected");
	int i;
	for(i=0;topics_pair[i].sub_topic!=NULL;i++){
		MQTT_Subscribe(client, topics_pair[i].sub_topic, topics_pair[i].sub_qos);
	}
	char data[]="Wireless Connected.";
	MQTT_DBG("MQTT: Publish:%s,length:%d",data,os_strlen(data));
	MQTT_Publish(client,mqtt_upload_topic,data,os_strlen(data),1,0);
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

	MQTT_DBG("Receive topic: %s, data: %s ", topicBuf, dataBuf);
	os_free(topicBuf);
	os_free(dataBuf);
	//undecision
	
	int i;
	for(i=0;topics_pair[i].sub_topic!=NULL;i++){
		mqtt_topic_pair *md = &topics_pair[i];
		//compare topic
		if(os_strncmp(topic,md->sub_topic,strlen(md->sub_topic))==0){
			mqtt_operator_api(data,data_len,md->pub_topic,md->pub_qos);
		}
	}
}


int ICACHE_FLASH_ATTR mqtt_is_connected(){
	//enable reconnect
	wifi_reconnect_count = 0;
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
	MQTT_InitConnection(&mqtt_client, MQTT_SERVER_IP, MQTT_SERVER_PORT, 0);
	MQTT_InitClient(&mqtt_client, "ThisIsClientId", "", "", 120, 1);
	//aliyun_mqtt_init();
	//MQTT_InitConnection(&mqtt_client, g_aliyun_mqtt.host, g_aliyun_mqtt.port, 0);
	//MQTT_InitClient(&mqtt_client, g_aliyun_mqtt.client_id, g_aliyun_mqtt.username,g_aliyun_mqtt.password, g_aliyun_mqtt.keepalive, 1);

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
