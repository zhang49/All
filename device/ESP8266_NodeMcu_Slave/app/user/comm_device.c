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

os_timer_t statusTimer;
int temperature_read_tick;

u8 DHT22MacAddr[6]  = {0xA2, 0x11, 0xA6, 0x55, 0x55, 0x55};
u8 Realy1MacAddr[6] = {0xA2, 0x11, 0xA6, 0x66, 0x66, 0x66};
u8 Realy2MacAddr[6] = {0xA2, 0x33, 0xA6, 0x55, 0x55, 0x55};
u8 Realy3MacAddr[6] = {0xA2, 0x44, 0xA6, 0x55, 0x55, 0x55};

static void deviceStatusTimerCb(void *arg){

#ifdef DHT22_OPEN
	dht22_read(&dhtData);
#elif defined RELAY1_OPEN
#elif defined RELAY2_OPEN
#elif defined RELAY3_OPEN
#endif

}

void comm_device_init(void){
int op=0;
#ifdef DHT22_OPEN

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
	os_timer_arm(&statusTimer, 8000, 1);
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



