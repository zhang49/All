/*
 * door_uart.h
 *
 *  Created on: Feb 26, 2019
 *      Author: root
 */

#ifndef COMM_UART_H_
#define COMM_UART_H_

#include <sys/types.h>
#include "c_stdint.h"


typedef struct {
	u8 xorCheck;
	u8 type;
	u8 mac_index;	//mac index
	u8 data[5];
	u8 cursor;
}UartRecvBuf;

typedef enum  {
	URS_Head1,
	URS_Head2,
	URS_Type,
	URS_Data,
	URS_MacIndex,
	URS_XorCheck
}UartRecvState;


void ICACHE_FLASH_ATTR comm_uart_init();
void ICACHE_FLASH_ATTR send_message_to_master(uint8_t mac_index,uint8_t type,uint8 *data);
void ICACHE_FLASH_ATTR uart_recv_passcheck();

void ICACHE_FLASH_ATTR uart_recv_callback(uint8_t *data,int len);

#endif /* DOOR_UART_H_ */
