#include "c_types.h"
#include "c_string.h"
#include "c_stdio.h"
#include "user_interface.h"
#include "mem.h"
#include "osapi.h"
#include "user_config.h"
#include "espconn.h"

#include "mqtt.h"
#include "mqtt_msg.h"

#include "driver/relay.h"
#include "mqtt/mqtt_door.h"

#include "sensor/sensors.h"

#include "serial_number.h"

#include "json/cJson.h"

#define MQTT_SERVER_IP "192.168.20.2"
#define MQTT_RELAY_SET_SUFIX "/SET"

extern uint8 remarkId_set;

static os_timer_t mqtt_timer;

static os_timer_t mqtt_test_timer;

static MQTT_Client mqtt_client;

static os_timer_t sensor_read_timer;

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
		{"/mymqtt/token/request",	0, 		"/mymqtt/token/response",	0},
		{"/mymqtt/test_say",		0,		NULL,						0},
		{NULL,						0,		NULL,						0}
};


typedef void(*mqtt_topic_callback)(MQTT_Client *client);

typedef struct{
	const char *type;
	mqtt_topic_callback func;
	int flag;
}door_def;

static door_def door_operator[]={
		{"GetWiFiConfig",			mqtt_wifi_config_read,				NULL},
		{"GetSafeConfig",			mqtt_safe_config_read,				NULL},
		{"GetDoorConfig",			mqtt_door_config_read,				NULL},
		{"SetWiFiConfig",			mqtt_wifi_config_write,				NULL},
		{"SetSafeConfig",			mqtt_door_expect_ret,				NULL},
		{"SetDoorConfig",			mqtt_door_expect_ret,				NULL},
		{"Command",					mqtt_door_expect_ret,				NULL},
		{NULL,						mqtt_door_expect_ret,				NULL}
};


static void mqtt_relay_change_cb(int id,int state){

	MQTT_DBG("mqtt_relay_change_cb #%d %d",id,state);

	mqtt_relay * relay = &relays[id];	

	if(state<0)
		state = relay_get_state(id);

	MQTT_Publish(&mqtt_client, relay->topic, state?"1":"0", 1, 0, 1);

}

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

static void mqtt_test_timer_cb(MQTT_Client* client)
{
	char data[]="mqtt_test";
	MQTT_DBG("MQTT: Publish:%s,length:%d",data,os_strlen(data));
	MQTT_Publish(client,"/mymqtt/set",data,os_strlen(data),1,0);
}

static void mqttConnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	MQTT_DBG("MQTT: Connected");
	/*int i;
	for(i=0;i<relay_count();i++){

		char * setTopic = (char *)os_zalloc(strlen(relays[i].topic)+strlen(MQTT_RELAY_SET_SUFIX)+1);
		os_strcpy(setTopic,relays[i].topic);
		os_strcat(setTopic,MQTT_RELAY_SET_SUFIX);

		MQTT_Subscribe(client, setTopic, 1);

		os_free(setTopic);

		mqtt_relay_change_cb(i,-1); //force 1st publish
	}*/
	int i;
	for(i=0;door_topics[i].sub_topic!=NULL;i++){
		MQTT_Subscribe(client, door_topics[i].sub_topic, door_topics[i].sub_qos);
	}
	char data[]="Client Connected";
	MQTT_DBG("MQTT: Publish:%s,length:%d",data,os_strlen(data));
	MQTT_Publish(client,"/mymqtt/test_say",data,os_strlen(data),1,0);

	//arm mqtt timer
	os_memset(&mqtt_timer,0,sizeof(os_timer_t));
	os_timer_disarm(&mqtt_timer);
	os_timer_setfn(&mqtt_timer, (os_timer_func_t *)mqtt_test_timer_cb, client);
	os_timer_arm(&mqtt_timer, 2000, 1);

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


	int i;
	for(i=0;door_topics[i].sub_topic!=NULL;i++){
		mqtt_door *md = &door_topics[i];
		//compare topic
		if(os_strncmp(topic,md->sub_topic,strlen(md->sub_topic))==0){
			//parse json
			cJSON *root = cJSON_Parse(data);
			if(root==NULL){
				uart_write_string(0,"http_door_command_api_read cJSON_Parse error\r\n");
				cJSON_Delete(root);
				return;
			}
			cJSON *type = cJSON_GetObjectItem(root,"type");
			if(type==NULL){
				uart_write_string(0,"http_door_command_api_read cJSON_GetObjectItem error\r\n");
				cJSON_Delete(root);
				return;
			}
			NODE_DBG("request type :%s",type->valuestring);
			int index=0;
			for(index=0;door_operator[index].type!=NULL;index++){
				if(strcmp(type->valuestring,door_operator[index].type)==0){
					cJSON_Delete(root);
					remarkId_set++;
					mqtt_door_comm_data *comm_data=(mqtt_door_comm_data*)os_malloc(sizeof(mqtt_door_comm_data));
					comm_data->tickcount = 0;
					comm_data->sub_topic = topic;
					comm_data->pub_topic = door_topics[i].pub_topic;
					comm_data->msg=data;
					client->user_data=comm_data;
					door_operator[index].func(client);
					return;
				}
			}
			cJSON_Delete(root);
		}
	}
	// set relay
	/*
	int i;
	for(i=0;i<relay_count();i++){

		mqtt_relay *r = &relays[i];

		//compare topic
		if(os_strncmp(topic,r->topic,strlen(r->topic))==0){
			MQTT_DBG("Topic 1st part match");
			//check set suffix
			if(os_strncmp(topic+strlen(r->topic),MQTT_RELAY_SET_SUFIX,strlen(MQTT_RELAY_SET_SUFIX))==0)
			{
				MQTT_DBG("Topic 2nd part match");
				int state = data[0] - '0';

				if(state==0 || state == 1)
				{
					MQTT_DBG("Setting relay #%d to %d via MQTT", i, state);
					relay_set_state(i,state);
					break;
				}

				
			}
		} 
			

	}
*/
	
}

static void ICACHE_FLASH_ATTR sensor_read_timer_cb(void *arg){

	char * buff = (char *)os_zalloc(64);

	sensor_data data;
	sensors_get_data(&data);	

	c_sprintf(buff,"%f",data.dht22.temp);
	MQTT_Publish(&mqtt_client, "temperature/"SERIAL_NUMBER, buff, strlen(buff), 0, 1);

	os_memset(buff,0,64);
	c_sprintf(buff,"%f",data.dht22.hum);
	MQTT_Publish(&mqtt_client, "humidity/"SERIAL_NUMBER, buff, strlen(buff), 0, 1);

	os_memset(buff,0,64);
	c_sprintf(buff,"%d",data.bmp180.press);
	MQTT_Publish(&mqtt_client, "pressure/"SERIAL_NUMBER, buff, strlen(buff), 0, 1);

	os_free(buff);



}

void ICACHE_FLASH_ATTR mqtt_app_init(){
	

	//set relays
	/*int i;
	for(i=0;i<relay_count();i++){
		relays[i].id=i;		
		os_strcpy(relays[i].topic,"relay/");
		os_strcat(relays[i].topic,SERIAL_NUMBER"/");
		os_strcati(relays[i].topic,i);
	}*/

	//register listeners 
	//relay_register_listener(mqtt_relay_change_cb);

	MQTT_InitConnection(&mqtt_client, MQTT_SERVER_IP, 1883, 0);
	MQTT_InitClient(&mqtt_client, SERIAL_NUMBER, MQTT_USER_NAME, MQTT_PASSWORD, 120, 1);

	MQTT_OnConnected(&mqtt_client, mqttConnectedCb);
	MQTT_OnDisconnected(&mqtt_client, mqttDisconnectedCb);
	MQTT_OnPublished(&mqtt_client, mqttPublishedCb);
	MQTT_OnData(&mqtt_client, mqttDataCb);
	
	//arm mqtt timer
	os_memset(&mqtt_timer,0,sizeof(os_timer_t));
	os_timer_disarm(&mqtt_timer);
	os_timer_setfn(&mqtt_timer, (os_timer_func_t *)mqtt_app_timer_cb, NULL);
	os_timer_arm(&mqtt_timer, 5000, 0);

	//arm sensor read timer
	/*os_memset(&sensor_read_timer,0,sizeof(os_timer_t));
	os_timer_disarm(&sensor_read_timer);
	os_timer_setfn(&sensor_read_timer, (os_timer_func_t *)sensor_read_timer_cb, NULL);
	os_timer_arm(&sensor_read_timer, 5000, 1);
	*/

}
