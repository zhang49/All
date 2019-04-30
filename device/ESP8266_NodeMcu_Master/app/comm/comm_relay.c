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
#include "espnow.h"

CommRelay relay[RELAYSIZE]={
		{RCT_EspNow,		0,		COMM_NOREFRESH},
		{RCT_EspNow,		0,		COMM_NOREFRESH},
		{RCT_EspNow,		0,		COMM_NOREFRESH}
};

void comm_relay_init(){

}

//current Only for relay1
void ICACHE_FLASH_ATTR comm_relay_status_set_app_api(int index,u8 status){
	os_printf("relay_index:%d,status:%d\r\n",index,status);
	relay[index].status = status;
	relay[index].refreshStatus = COMM_NOREFRESH;
	if(relay[index].con_type == RCT_EspNow){
		u8 data[5]={status,0,0,0,0};
		int mac_index;
		switch(index){
		case 0:
			mac_index=RELAY1_MACINDEX;
			break;
		case 1:
			mac_index=RELAY2_MACINDEX;
			break;
		case 2:
			mac_index=RELAY3_MACINDEX;
			break;
		default:
			return;
			break;
		}
		send_message_to_slave(mac_index,RequestRelay,data);
	}
}

void ICACHE_FLASH_ATTR comm_relay_status_set_inner_api(u8 mac_index,u8 status){
	os_printf("CRSSIA: mad_index:%d\r\n",mac_index);
	int index;
	switch(mac_index){
	case RELAY1_MACINDEX:
		index=0;
		break;
	case RELAY2_MACINDEX:
		index=1;
		break;
	case RELAY3_MACINDEX:
		index=2;
		break;
	default:
		return;
		break;
	}
	relay[index].status = status;
	relay[index].refreshStatus = COMM_REFRESHED;
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


