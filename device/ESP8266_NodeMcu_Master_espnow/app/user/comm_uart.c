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
#include "comm_uart.h"

static UartRecvBuf uart_recv_buf={0};

static UartRecvState uRS=URS_Head1;

void ICACHE_FLASH_ATTR comm_uart_init()
{
	//fro test;
	uart_register_data_callback(uart_recv_callback);
}

void ICACHE_FLASH_ATTR uart_recv_callback(uint8_t *bytes,int len)
{
	u8 i=0;
	for(i=0;i<len;i++){
restart:
		if(uRS!=URS_XorCheck)
			uart_recv_buf.xorCheck^=bytes[i];
		switch(uRS){
			case URS_Head1:
				if(bytes[i]==0xaa){
					uRS=URS_Head2;
					uart_recv_buf.xorCheck=bytes[i];
				}else uRS=URS_Head1;
				break;
			case URS_Head2:
				if(bytes[i]==0xbb){
					uRS=URS_MacIndex;
				}else {
					uRS=URS_Head1;
					goto restart;
				}
				break;
			case URS_MacIndex:
				uRS=URS_Type;
				uart_recv_buf.mac_index=bytes[i];
				uart_recv_buf.cursor=0;
				break;
			case URS_Type:
				uRS=URS_Data;
				uart_recv_buf.type=bytes[i];
				break;
			case URS_Data:
				if(uart_recv_buf.cursor<5){
					uart_recv_buf.data[uart_recv_buf.cursor]=bytes[i];
					uart_recv_buf.cursor++;
					if(uart_recv_buf.cursor==5)
						uRS=URS_XorCheck;
				}
				break;
			case URS_XorCheck:
				uRS=URS_Head1;
				if(uart_recv_buf.xorCheck==bytes[i]){
					uart_recv_passcheck();
				}else{
					NODE_DBG("ESPNOW: Don't pass check");
					goto restart;
				}
				break;
		}
	}
	return;
}

void ICACHE_FLASH_ATTR uart_recv_passcheck()
{
	esp_now_send_api(getMacAddrByIndex(uart_recv_buf.mac_index),
			uart_recv_buf.type,uart_recv_buf.data);
}

void ICACHE_FLASH_ATTR send_message_to_master(uint8_t mac_index,uint8_t type,uint8 *data)
{
	uint8 os[10];
	os_memset(os,0,10);
	int cursor=0,i=0;
	uint8 xorCheck=0;
	*(os+cursor++)=0xaa;
	*(os+cursor++)=0xbb;
	*(os+cursor++)=mac_index;
	*(os+cursor++)=type;
	for(i=0;i<5;i++)*(os+cursor++)=data[i];
	for(i=0;i<cursor;i++)xorCheck^=*(os+i);
	*(os+cursor)=xorCheck;//TotalCheck
	for(i=0;i<=cursor;i++){
		uart1_write_char(*(os+i));
	}
}



