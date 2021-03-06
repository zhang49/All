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

#include "driver/relay.h"
#include "door/door.h"

#include "sensor/sensors.h"

#include "serial_number.h"

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
}mqtt_door;


static mqtt_door door_topics[]={
		{"autodoor/config/"DEVICE_TOKEN,	0, 			NULL,	0},
		{"autodoor/control/"DEVICE_TOKEN,	0, 			NULL,	0},
		{NULL,								0,			NULL,	0}
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
	for(i=0;door_topics[i].sub_topic!=NULL;i++){
		MQTT_Subscribe(client, door_topics[i].sub_topic, door_topics[i].sub_qos);
	}
	/*
	char data[]="Client Connected";
	MQTT_DBG("MQTT: Publish:%s,length:%d",data,os_strlen(data));
	MQTT_Publish(client,"/mymqtt/test_say",data,os_strlen(data),1,0);
	*/
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
	/*
	int i;
	for(i=0;door_topics[i].sub_topic!=NULL;i++){
		mqtt_door *md = &door_topics[i];
		//compare topic
		if(os_strncmp(topic,md->sub_topic,strlen(md->sub_topic))==0){
			mqtt_door_comm_data *comm_data=(mqtt_door_comm_data*)os_malloc(sizeof(mqtt_door_comm_data));
			client->sign=CLIENT_IS_MQTT;
			comm_data->sub_topic = topic;
			comm_data->pub_topic = md->pub_topic;
			comm_data->sub_qos=md->sub_qos;
			comm_data->pub_qos=md->pub_qos;
			comm_data->data_len=data_len;
			comm_data->msg=data;
			client->user_data=comm_data;
			mqtt_operator_api(client);
		}
	}
	*/
}

void ICACHE_FLASH_ATTR mqtt_app_init(){
	MQTT_InitConnection(&mqtt_client, MQTT_SERVER_IP, MQTT_SERVER_PORT, 0);
	MQTT_InitClient(&mqtt_client, SERIAL_NUMBER, MQTT_USER_NAME, MQTT_PASSWORD, 120, 1);

	MQTT_OnConnected(&mqtt_client, mqttConnectedCb);
	MQTT_OnDisconnected(&mqtt_client, mqttDisconnectedCb);
	MQTT_OnPublished(&mqtt_client, mqttPublishedCb);
	MQTT_OnData(&mqtt_client, mqttDataCb);
	
	//arm mqtt timer
	os_memset(&mqtt_timer,0,sizeof(os_timer_t));
	os_timer_disarm(&mqtt_timer);
	os_timer_arm(&mqtt_timer, 5000, 1);
	os_timer_setfn(&mqtt_timer, (os_timer_func_t *)mqtt_app_timer_cb, NULL);

}
