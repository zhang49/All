/*
 * comm_relay.c
 *
 *  Created on: Mar 19, 2019
 *      Author: root
 */

#include "osapi.h"
#include "user_interface.h"
#include "gpio.h"
#include "user_config.h"
#include "comm/comm_pub_def.h"
#include "comm/comm_relay.h"
#include "comm/comm_espnow.h"
#include "espnow.h"

CommRelay relay[RELAYSIZE]={
		{RCT_EspNow,	{0xA2, 0x11, 0xA6, 0x66, 0x66, 0x66},	0,	COMM_NOREFRESH},
		{RCT_None,		{0xA2, 0x33, 0xA6, 0x55, 0x55, 0x55},	0,	COMM_NOREFRESH},
		{RCT_None,		{0xA2, 0x44, 0xA6, 0x55, 0x55, 0x55},	0,	COMM_NOREFRESH}
};

void comm_relay_init(){
	int i;
	for(i=0;i<RELAYSIZE;i++){
		if(relay[i].con_type==RCT_EspNow){
			esp_now_add_peer(relay[i].mac_addr, ESP_NOW_ROLE_SLAVE, EspNowChannel, NULL, 0);
		}
	}
}

//current Only for relay1
void ICACHE_FLASH_ATTR comm_relay_status_set_app_api(int index,u8 status){
	//relay[index].status = status;
	relay[index].refreshStatus = COMM_NOREFRESH;
	if(relay[index].con_type == RCT_EspNow){
		u8 data[5]={status,0,0,0,0};
		esp_now_send_api(relay[index].mac_addr,RequestRelay,data);
	}
}

void ICACHE_FLASH_ATTR comm_relay_status_set_inner_api(u8 *mac_addr,u8 status){
	int i;
	for(i=0;i<RELAYSIZE;i++){

		if(relay[i].con_type==RCT_EspNow && os_memcmp(relay[i].mac_addr,mac_addr)==0){
			relay[i].status = status;
			relay[i].refreshStatus = COMM_REFRESHED;
		}
	}
}

int ICACHE_FLASH_ATTR comm_relay_status_get_api(int index){
	return relay[index].status;
}


int ICACHE_FLASH_ATTR comm_relay_refresh_status_get(int index){
	return relay[index].refreshStatus;
}

void ICACHE_FLASH_ATTR comm_relay_refresh_set(int index,int status){
	relay[index].refreshStatus=status;
}


