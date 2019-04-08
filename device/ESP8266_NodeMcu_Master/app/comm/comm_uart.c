/*
 * door_uart.c
 *
 *  Created on: Feb 26, 2019
 *      Author: root
 */
#include "comm_uart.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"

#include "osapi.h"

#include "user_config.h"

#include "driver/uart.h"
#include "comm/comm_pub_def.h"
#include "comm/comm_uart.h"
#include "http/app.h"

#include "json/cJson.h"

enum UartRecvState ur_state=URS_FLAGE1;
static uart_recv uart_recv_raw={0,0,0,0,0,NULL};
static door_comm_buf comm_buf;

void ICACHE_FLASH_ATTR comm_uart_init()
{
	//fro test;
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
				ur_state=URS_LNE_H;
				uart_recv_raw.totalCheck+=*(data+i);
			}
			else{
				ur_state=URS_FLAGE1;
				uart_recv_raw.totalCheck=0;
				goto restart;
			}
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
	master_msg_type=uart_recv_raw.type;
	data_save *ds_ptr=NULL;
	switch(master_msg_type){
	case NORMAL_CONFIG:
		ds_ptr=&comm_buf.config;
		break;
	case SYS_COMMAND:
		break;
	case SYN_CONTROL:
		ds_ptr=&comm_buf.syn_control;
		break;
	case SYN_STATE:
		break;
		/*
		syn_state.sm_state=*(uart_recv_raw.data+0);
		syn_state.comm_state=*(uart_recv_raw.data+1);
		syn_state.temperature=*(uart_recv_raw.data+2);
		syn_state.wetness=*(uart_recv_raw.data+3);
		syn_state.power=*(uart_recv_raw.data+4);
		syn_state.run_time=*(uart_recv_raw.data+5);
		break;
		*/
	}
	if(ds_ptr!=NULL){
		ds_ptr->refresh_state=COMM_REFRESHED;
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
	return;
}

void ICACHE_FLASH_ATTR send_message_to_master(enum MasterMsgType msg_type,uint8 *data,int len)
{
	len=len+HEADLENGTH+1;
	uint8 *os=(uint8 *)os_malloc(len);
	os_memset(os,0,len);
	int cursor=0,i=0;
	uint8 totalCheck=0;
	*(os+cursor++)='Z';//Flag
	*(os+cursor++)='Y';
	*(os+cursor++)=len>>8;//Length_H
	*(os+cursor++)=len&0x00ff;//Length_L
	*(os+cursor++)=msg_type;//Type
	for(i=0;i<cursor;i++)totalCheck+=*(os+i);
	for(i=0;i<len-HEADLENGTH-1;i++){
		*(os+cursor++)=*(data+i);//Data
		totalCheck+=*(data+i);
	}
	*(os+cursor)=totalCheck;//TotalCheck
	for(i=0;i<len;i++){
		uart0_write_char(*(os+i));
	}
	os_free(os);
}

int ICACHE_FLASH_ATTR config_refresh_get(){
	return comm_buf.config.refresh_state;
}
void ICACHE_FLASH_ATTR door_config_refresh_set(uint8 status){
	comm_buf.config.refresh_state=status;
}
int ICACHE_FLASH_ATTR door_config_refresh_get(){
	return comm_buf.config.refresh_state;
}
char* ICACHE_FLASH_ATTR door_config_read(){
	return comm_buf.config.buf;
}


void ICACHE_FLASH_ATTR door_config_write_status_set(uint8 status){
	comm_buf.door_config_write_state=status;
}
int ICACHE_FLASH_ATTR door_config_write_status_get(){

}

void ICACHE_FLASH_ATTR syn_control_refresh_set(uint8 status){
	comm_buf.syn_control.refresh_state=status;
}
int ICACHE_FLASH_ATTR syn_control_refresh_get(){
	return comm_buf.syn_control.refresh_state;
}
char* ICACHE_FLASH_ATTR syn_control_read(){
	return comm_buf.syn_control.buf;
}




