/*
 * comm_relay.h
 *
 *  Created on: Mar 19, 2019
 *      Author: root
 */

#ifndef COMM_RELAY_H_
#define COMM_RELAY_H_

#define RELAY1_PIN 4
#define RELAY2_PIN 5

#define RELAYSIZE 4

typedef enum {
	RCT_EspNow,
	RCT_SelfGpio,
	RCT_Net,
	RCT_None
}RelayConType;

typedef struct {
	RelayConType con_type;
	//u8 mac_addr[6];
	int status;
	int refreshStatus;
}CommRelay;

void ICACHE_FLASH_ATTR comm_relay_status_set_app_api(int index,u8 status);
void ICACHE_FLASH_ATTR comm_relay_status_set_inner_api(u8 mac_index,u8 status);
int  ICACHE_FLASH_ATTR comm_relay_status_get_api(int index);
void ICACHE_FLASH_ATTR comm_relay_status_set_api(int index,int status);
int  ICACHE_FLASH_ATTR comm_relay_refresh_status_get(int index);
void ICACHE_FLASH_ATTR comm_relay_refresh_set(int index,int status);

#endif /* COMM_RELAY_H_ */




