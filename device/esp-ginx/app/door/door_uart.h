/*
 * door_uart.h
 *
 *  Created on: Feb 26, 2019
 *      Author: root
 */

#ifndef DOOR_UART_H_
#define DOOR_UART_H_


#include <sys/types.h>
#include "c_stdint.h"

#include "http/http.h"

#define HEADLENGTH 6
#define BUF_MAXSIZE 3
enum UartRecvState{
	URS_FLAGE1,
	URS_FLAGE2,
	URS_REMARK,
	URS_LNE_H,
	URS_LNE_L,
	URS_TYPE,
	URS_DATA,
	URS_CHECK
};

typedef struct {
	uint8 totalCheck;
	uint8 remarkId;
	uint8 type;
	uint16 length;
	uint16 cursor;
	uint8 *data;
}uart_recv;

typedef struct {
	uint8 *buf;
	uint16 length;
	union{
		int refresh_state;//0 :no refresh,1 :refreshing,2 :refresh success
		uint8 remarkId;
	};
}data_save;
typedef struct {
	data_save door_config;
	data_save safe_config;
	data_save others[BUF_MAXSIZE];

}door_conf_buf;
typedef struct {
	int remarkId;
	cgi_execute_function execute;

}need_send_cgi;
//////////////////////////////////remarkId, buf
void ICACHE_FLASH_ATTR door_uart_init();
void ICACHE_FLASH_ATTR send_message_to_master(uint8 remarkId,uint8 type,uint8 *data,int len);
void ICACHE_FLASH_ATTR uart_recv_callback(uint8_t *data,int len);
void ICACHE_FLASH_ATTR uart_recv_passcheck();



#endif /* DOOR_UART_H_ */
