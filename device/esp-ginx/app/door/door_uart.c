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
uart_recv uart_recv_raw={0,0,0,0,0,NULL};
door_conf_buf configbuf;
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
	if(root==NULL)
	{
		NODE_DBG("uart cJSON_Parse error");
		goto badrecv;
	}

	cJSON *type = cJSON_GetObjectItem(root,"type");
	if(type==NULL)
	{
		NODE_DBG("uart cJSON_GetObjectItem error");
		goto badrecv;
	}
	NODE_DBG("uart type :%s",type->valuestring);
	if(uart_recv_raw.type==0x01){	//master command

	}else if(uart_recv_raw.type==0x02){	//master net data
		if(os_strcmp(type->valuestring,"Reply_GetSafeConfig")==0){
			if(configbuf.safe_config.buf==NULL)
				configbuf.safe_config.buf=(uint8 *)os_malloc(uart_recv_raw.length-HEADLENGTH);
			else if(configbuf.safe_config.length!=uart_recv_raw.length-HEADLENGTH-1)
				configbuf.safe_config.buf=(uint8 *)os_realloc(configbuf.safe_config.buf,uart_recv_raw.length-HEADLENGTH);

			configbuf.safe_config.length=uart_recv_raw.length-HEADLENGTH-1;
			int i=0;
			for(i=0;i<configbuf.safe_config.length;i++)
				*(configbuf.safe_config.buf+i)=*(uart_recv_raw.data+i);
			*(configbuf.safe_config.buf+i)=0;
			configbuf.safe_config.refresh_state=2;
			NODE_DBG("uart recv SafeConfig:%s",configbuf.safe_config.buf);

		}else if(os_strcmp(type->valuestring,"Reply_GetDoorConfig")==0){
			if(configbuf.door_config.buf==NULL)
				configbuf.door_config.buf=(uint8 *)os_malloc(uart_recv_raw.length-HEADLENGTH);
			else if(configbuf.door_config.length!=uart_recv_raw.length-HEADLENGTH-1)
				configbuf.door_config.buf=(uint8 *)os_realloc(configbuf.door_config.buf,uart_recv_raw.length-HEADLENGTH);

			configbuf.door_config.length=uart_recv_raw.length-HEADLENGTH-1;
			int i=0;
			for(i=0;i<configbuf.door_config.length;i++)
				*(configbuf.door_config.buf+i)=*(uart_recv_raw.data+i);
			*(configbuf.door_config.buf+i)=0;
			configbuf.door_config.refresh_state=2;
			NODE_DBG("uart recv DoorConfig:%s",configbuf.door_config.buf);
		}else{
			int index=0,isfull=1;
			for(index=0;index<BUF_MAXSIZE;index++){
				if(configbuf.others[index].remarkId==NULL || configbuf.others[index].remarkId==0){
					isfull=0;
					break;
				}
			}
			if(isfull)index=0;//cover the oldest solt

			configbuf.others[index].length=uart_recv_raw.length-HEADLENGTH-1;
			if(configbuf.others[index].buf==NULL)
				configbuf.others[index].buf=(uint8 *)os_malloc(uart_recv_raw.length-HEADLENGTH);
			else if(configbuf.others[index].length!=uart_recv_raw.length-HEADLENGTH-1)
				configbuf.others[index].buf=(uint8 *)os_realloc(configbuf.others[index].buf,uart_recv_raw.length-HEADLENGTH);
			configbuf.others[index].length=uart_recv_raw.length-HEADLENGTH-1;
			int i=0;
			for(i=0;i<configbuf.others[index].length;i++)
				*(configbuf.others[index].buf+i)=*(uart_recv_raw.data+i);
			*(configbuf.others[index].buf+i)=0;
			configbuf.others[index].remarkId=uart_recv_raw.remarkId;
		}
	}
badrecv:
	cJSON_Delete(root);
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

