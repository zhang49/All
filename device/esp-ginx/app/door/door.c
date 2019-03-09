/*
 * door.c
 *
 *  Created on: Feb 26, 2019
 *      Author: root
 */

#include "door.h"

#include "door_uart.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"

#include "osapi.h"

#include "user_config.h"
#include "osapi.h"

#include "user_interface.h"

#include "door/door_uart.h"
#include "http/app.h"

uint8 remarkId_set = 0;

extern door_conf_buf configbuf;

os_timer_t doorRequestTimer;

void ICACHE_FLASH_ATTR door_init(){
	door_uart_init();
	os_memset(&doorRequestTimer,0,sizeof(os_timer_t));
	os_timer_disarm(&doorRequestTimer);
	os_timer_setfn(&doorRequestTimer, (os_timer_func_t *)door_request_all_config, NULL);
	os_timer_arm(&doorRequestTimer, 500, 1);
}

int ICACHE_FLASH_ATTR safe_config_isrefresh(){
	return configbuf.safe_config.refresh_state==2?1:0;
}
int ICACHE_FLASH_ATTR door_config_isrefresh(){
	return configbuf.door_config.refresh_state==2?1:0;
}
int ICACHE_FLASH_ATTR door_config_refresh_set(uint8 status){
	return configbuf.door_config.refresh_state=status;
}
char* ICACHE_FLASH_ATTR get_safe_config(){
	return configbuf.safe_config.buf;
}
char* ICACHE_FLASH_ATTR get_door_config(){
	return configbuf.door_config.buf;
}

void request_safe_config(){
	char resData[]="{\"type\":\"GetSafeConfig\",\"data\":\"\"}";
	send_message_to_master(0,0x02,resData,os_strlen(resData));
}

void request_door_config(){
	char resData[]="{\"type\":\"GetDoorConfig\",\"data\":\"\"}";
	send_message_to_master(0,0x02,resData,os_strlen(resData));
}

void ICACHE_FLASH_ATTR door_request_all_config(){
	if(configbuf.safe_config.refresh_state==0)configbuf.safe_config.refresh_state++;
	else if(configbuf.safe_config.refresh_state==1)request_safe_config();
	if(configbuf.door_config.refresh_state==0)configbuf.door_config.refresh_state++;
	else  if(configbuf.door_config.refresh_state==1)request_door_config();
}



int ICACHE_FLASH_ATTR door_databuf_remarkid_isExist(uint8 remarkId){
	int pos=0;
	for(pos=0;pos<BUF_MAXSIZE;pos++){
		if((configbuf.others+pos)->remarkId==remarkId)return 1;
	}
	return 0;
}
char* ICACHE_FLASH_ATTR door_others_read(uint8 remarkId){
	int pos=0;
	for(pos=0;pos<BUF_MAXSIZE;pos++){
		if((configbuf.others+pos)->remarkId==remarkId)return (configbuf.others+pos)->buf;
	}
	return NULL;
}
int ICACHE_FLASH_ATTR door_others_getlength(uint8 remarkId){
	int pos=0;
	for(pos=0;pos<BUF_MAXSIZE;pos++){
		if((configbuf.others+pos)->remarkId==remarkId)return (configbuf.others+pos)->length;
	}
	return 0;
}





