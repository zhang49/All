/*
 * door_uart.c
 *
 *  Created on: Feb 26, 2019
 *      Author: root
 */
#include "door_uart.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"

#include "osapi.h"

#include "user_config.h"

#include "driver/uart.h"

#include "door/door_uart.h"
#include "http/app.h"

#include "json/cJson.h"

enum UartRecvState ur_state=URS_FLAGE1;
static uart_recv uart_recv_raw={0,0,0,0,0,NULL};
static door_conf_buf configbuf;
static uint8 others_buf_cursor = 0;

void ICACHE_FLASH_ATTR door_uart_init()
{
	uart_register_data_callback(uart_recv_callback);
}
void ICACHE_FLASH_ATTR uart_recv_callback(uint8_t *data,int len)
{
	int i=0;
	*(data+len)=0;
	//NODE_DBG("uart recv:%s",data);
	while(i<len)
	{
restart:
		switch(ur_state)
		{
		case URS_FLAGE1:
			if(*(data+i)=='Z'){
				ur_state=URS_FLAGE2;
				uart_recv_raw.totalCheck=*(data+i);
			}
			break;
		case URS_FLAGE2:
			if(*(data+i)=='Y'){
				ur_state=URS_REMARK;
				uart_recv_raw.totalCheck+=*(data+i);
			}
			else{
				ur_state=URS_FLAGE1;
				uart_recv_raw.totalCheck=0;
				goto restart;
			}
			break;
		case URS_REMARK:
			ur_state=URS_LNE_H;
			uart_recv_raw.totalCheck+=*(data+i);
			uart_recv_raw.remarkId=*(data+i);
			break;
		case URS_LNE_H:
			ur_state=URS_LNE_L;
			uart_recv_raw.totalCheck+=*(data+i);
			uart_recv_raw.length=*(data+i);
			if(uart_recv_raw.length==0xff)
				uart_recv_raw.length=0;
			break;
		case URS_LNE_L:
			ur_state=URS_TYPE;
			uart_recv_raw.totalCheck+=*(data+i);
			uart_recv_raw.length<<=8;
			uart_recv_raw.length|=*(data+i);
			if(uart_recv_raw.length>3096){
				ur_state=URS_FLAGE1;
				uart_recv_raw.length=0;
				uart_recv_raw.totalCheck=0;
				goto restart;
			}
			break;
		case URS_TYPE:
			ur_state=URS_DATA;
			uart_recv_raw.type=*(data+i);
			uart_recv_raw.totalCheck+=*(data+i);

			NODE_DBG("uart length:%d,remarkId:%d,type:%d",uart_recv_raw.length,uart_recv_raw.remarkId,	uart_recv_raw.type);
			break;
		case URS_DATA:
			uart_recv_raw.totalCheck+=*(data+i);
			if(uart_recv_raw.cursor==0 && uart_recv_raw.length-HEADLENGTH-1<=0){
				ur_state=URS_FLAGE1;
				uart_recv_raw.length=0;
				uart_recv_raw.cursor=0;
				uart_recv_raw.totalCheck=0;
				goto restart;
			}
			if(uart_recv_raw.data==NULL){
				//malloc
				uart_recv_raw.cursor=0;
				uart_recv_raw.data=(uint8 *)os_malloc(uart_recv_raw.length-HEADLENGTH);
				//os_free(uart_recv_raw.data);
			}else if(uart_recv_raw.cursor==0){
				uart_recv_raw.data=(uint8 *)os_realloc(uart_recv_raw.data,uart_recv_raw.length-HEADLENGTH);
				NODE_DBG("uart length:%d,realloc memory",uart_recv_raw.length);
			}
			//malloc failed
			if(uart_recv_raw.data==NULL){
				ur_state=URS_FLAGE1;
				uart_recv_raw.length=0;
				uart_recv_raw.cursor=0;
				uart_recv_raw.totalCheck=0;
				goto restart;

			}

			*(uart_recv_raw.data+uart_recv_raw.cursor)=*(data+i);
			uart_recv_raw.cursor++;
			if(uart_recv_raw.cursor==uart_recv_raw.length-HEADLENGTH-1){
				*(uart_recv_raw.data+uart_recv_raw.cursor)=0;
				NODE_DBG("uart length:%d,data copy over:%s",uart_recv_raw.length,uart_recv_raw.data);
				NODE_DBG("i=%d,raw_data_length=%d",i,len);
				ur_state=URS_CHECK;
				break;
			}
			break;
		case URS_CHECK:
			if(uart_recv_raw.totalCheck==*(data+i)){
				NODE_DBG("pass check");
				uart_recv_passcheck();
			}else{
				ur_state=URS_FLAGE1;
				uart_recv_raw.cursor=0;
				uart_recv_raw.totalCheck=0;
			    NODE_DBG("didn't pass check");
				goto restart;
			}
			ur_state=URS_FLAGE1;
			uart_recv_raw.cursor=0;
			uart_recv_raw.totalCheck=0;
			break;
		}
		i++;
	}
	return;
}
void ICACHE_FLASH_ATTR uart_recv_passcheck()
{
	//parse json
	cJSON *root = cJSON_Parse(uart_recv_raw.data);
	if(root==NULL){
		NODE_DBG("uart cJSON_Parse error");
		goto badrecv;
	}
	cJSON *type = cJSON_GetObjectItem(root,"type");
	if(type==NULL){
		NODE_DBG("uart cJSON_GetObjectItem error");
		goto badrecv;
	}
	NODE_DBG("uart type :%s",type->valuestring);
	if(uart_recv_raw.type==0x01){	//master command

	}else if(uart_recv_raw.type==0x02){	//master net data
		data_save *ds_ptr=NULL;
		if(os_strcmp(type->valuestring,"Reply_GetSafeConfig")==0){
			ds_ptr=&configbuf.safe_config;
			ds_ptr->refresh_state=CONFIG_REFRESHED;
			NODE_DBG("uart recv Reply_GetDoorConfig");
		}else if(os_strcmp(type->valuestring,"Reply_GetDoorConfig")==0){
			ds_ptr=&configbuf.door_config;
			ds_ptr->refresh_state=CONFIG_REFRESHED;
			NODE_DBG("uart recv Reply_GetDoorConfig");
		}else{
			//when set some config,must change refresh state
			if(os_strcmp(type->valuestring,"Reply_SetDoorConfig")==0){
				door_config_refresh_set(0);
			}else if(os_strcmp(type->valuestring,"Reply_SetSafeConfig")==0){
				safe_config_refresh_set(0);
			}
			ds_ptr=&configbuf.others[others_buf_cursor%OTHERS_BUF_MAXSIZE];
			others_buf_cursor++;
			if(others_buf_cursor==0xff){
				others_buf_cursor=0xff%OTHERS_BUF_MAXSIZE;
			}
			ds_ptr->remarkId=uart_recv_raw.remarkId;
		}
		if(ds_ptr!=NULL){
			if(ds_ptr->buf==NULL)
				ds_ptr->buf=(uint8 *)os_malloc(uart_recv_raw.length-HEADLENGTH);
			else if(ds_ptr->length!=uart_recv_raw.length-HEADLENGTH-1)
				ds_ptr->buf=(uint8 *)os_realloc(ds_ptr->buf,uart_recv_raw.length-HEADLENGTH);

			ds_ptr->length=uart_recv_raw.length-HEADLENGTH-1;
			int i=0;
			for(i=0;i<ds_ptr->length;i++)
				*(ds_ptr->buf+i)=*(uart_recv_raw.data+i);
			*(ds_ptr->buf+i)=0;
		}
	}
badrecv:
	if(root!=NULL){
		cJSON_Delete(root);
	}
	return;

}

void ICACHE_FLASH_ATTR send_message_to_master(uint8 remarkId,uint8 type,uint8 *data,int len)
{
	len=len+HEADLENGTH+1;
	uint8 *os=(uint8 *)os_malloc(len);
	os_memset(os,0,len);
	int cursor=0,i=0;
	uint8 totalCheck=0;
	*(os+cursor++)='Z';
	*(os+cursor++)='Y';
	*(os+cursor++)=remarkId;//remarkId
	*(os+cursor++)=len>>8;//Length_H
	if(*(os+cursor)==0)*(os+cursor)=0xff;
	*(os+cursor++)=len&0x00ff;//Length_L
	*(os+cursor++)=type;//Type
	for(i=0;i<cursor;i++)totalCheck+=*(os+i);
	for(i=0;i<len-HEADLENGTH-1;i++){
		*(os+cursor++)=*(data+i);
		totalCheck+=*(data+i);
	}
	*(os+cursor)=totalCheck;
	for(i=0;i<len;i++){
		uart0_write_char(*(os+i));
	}
	os_free(os);
}



int ICACHE_FLASH_ATTR door_config_refresh_get(){
	return configbuf.door_config.refresh_state;
}
int ICACHE_FLASH_ATTR door_config_refresh_set(uint8 status){
	return configbuf.door_config.refresh_state=status;
}
char* ICACHE_FLASH_ATTR door_config_read(){
	return configbuf.door_config.buf;
}

int ICACHE_FLASH_ATTR safe_config_refresh_get(){
	return configbuf.safe_config.refresh_state;
}
int ICACHE_FLASH_ATTR safe_config_refresh_set(uint8 status){
	return configbuf.safe_config.refresh_state=status;
}
char* ICACHE_FLASH_ATTR safe_config_read(){
	return configbuf.safe_config.buf;
}

int ICACHE_FLASH_ATTR door_others_remarkid_isExist(uint8 remarkId){
	int pos=0;
	for(pos=0;pos<OTHERS_BUF_MAXSIZE;pos++){
		if((configbuf.others+pos)->remarkId==remarkId)return 1;
	}
	return 0;
}

char* ICACHE_FLASH_ATTR door_others_read(uint8 remarkId){
	int pos=0;
	for(pos=0;pos<OTHERS_BUF_MAXSIZE;pos++){
		if((configbuf.others+pos)->remarkId==remarkId)return (configbuf.others+pos)->buf;
	}
	return NULL;
}
int ICACHE_FLASH_ATTR door_others_getlength(uint8 remarkId){
	int pos=0;
	for(pos=0;pos<OTHERS_BUF_MAXSIZE;pos++){
		if((configbuf.others+pos)->remarkId==remarkId)return (configbuf.others+pos)->length;
	}
	return 0;
}

