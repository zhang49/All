/*
 * comm_device.c
 *
 *  Created on: Mar 26, 2019
 *      Author: root
 */
#include "comm_device.h"
#include "sensor/dht22.h"
#include "osapi.h"
#include "ets_sys.h"
#include "user_interface.h"
#include "user_config.h"
#include "espnow.h"

#include "gpio.h"

#include "comm_pub_def.h"

dht_data dhtData;
uint8_t ray_value;

os_timer_t statusTimer;
int timer_tick=0;
extern u8 ControllerMacAddr[6];

u8 DHT22MacAddr[6]  = {0xA2, 0x11, 0xA6, 0x55, 0x55, 0x55};
u8 Realy1MacAddr[6] = {0xA2, 0x11, 0xA6, 0x66, 0x66, 0x66};
u8 Realy2MacAddr[6] = {0xA2, 0x33, 0xA6, 0x55, 0x55, 0x55};
u8 Realy3MacAddr[6] = {0xA2, 0x44, 0xA6, 0x55, 0x55, 0x55};
u8 RayMacAddr[6] 	= {0xA2, 0x55, 0xA6, 0x55, 0x55, 0x55};

static void deviceStatusTimerCb(void *arg){
uint8 retdata[5];
#ifdef DHT22_OPEN
	timer_tick++;
	if(timer_tick <= 8)return;
	timer_tick=0;
	dht22_read(&dhtData);
	dht22_temperature_read_api(retdata);
	dht22_humidity_read_api(retdata+3);
	esp_now_send_api(ControllerMacAddr,ReplyDht22,retdata);
#elif defined RELAY1_OPEN
#elif defined RELAY2_OPEN
#elif defined RELAY3_OPEN
#elif defined RAY_OPEN
	ray_value=system_adc_read();
	retdata[0]=ray_value&0x0f;
	retdata[1]=ray_value>>8;
	esp_now_send_api(ControllerMacAddr,ReplyRay,retdata);
#endif
}

void comm_device_init(void){
int op=0;
#ifdef DHT22_OPEN
	dht22_init();
#elif defined RELAY1_OPEN
	op=1;
#elif defined RELAY2_OPEN
	op=1;
#elif defined RELAY3_OPEN
	op=1;
#endif

	switch(op){
	case 1:
		comm_relay_init();
		break;
	}
	os_memset(&statusTimer,0,sizeof(os_timer_t));
	os_timer_disarm(&statusTimer);
	os_timer_setfn(&statusTimer, (os_timer_func_t *)deviceStatusTimerCb, NULL);
	os_timer_arm(&statusTimer, 1000, 1);
}

void ICACHE_FLASH_ATTR comm_relay_init(){
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, RELAY_PIN);//选择GPIO0
}

void ICACHE_FLASH_ATTR comm_relay_status_set(u8 level){
	GPIO_OUTPUT_SET(RELAY_PIN, level);
}

void dht22_humidity_read_api(u8 *data){
	data[0]=((int)(dhtData.hum/1));
	data[1]=((int)((dhtData.hum-data[0])*100));
	NODE_DBG("Humidity    Integet:%d,Point:%d",data[0],data[1]);
}

void dht22_temperature_read_api(u8 *data){
	if(dhtData.temp<0){
		dhtData.temp*=-1;
		data[0]='-';
	}else{
		data[0]='+';
	}
	data[1]=((int)(dhtData.temp/1));
	data[2]=((int)((dhtData.temp-data[1])*100));
	NODE_DBG("Temperature sign:%c, Integet:%d, Point:%d",data[0],data[1],data[2]);
}

uint16_t ray_read_api(){
	return ray_value;
}

void motor_pos_api(u8 per){

}

void motor_pas_api(u8 per){

}



