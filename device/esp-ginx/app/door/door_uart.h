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
#define OTHERS_BUF_MAXSIZE 3

#define CONFIG_NOREFRESH 0
#define CONFIG_REFRESHING 1
#define CONFIG_REFRESHED 2

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
	data_save door_config;		//save door_config
	data_save safe_config;		//save safe_config
	data_save others[OTHERS_BUF_MAXSIZE]; //except door and safe config

}door_conf_buf;

//////////////////////////////////remarkId, buf
void ICACHE_FLASH_ATTR door_uart_init();
void ICACHE_FLASH_ATTR send_message_to_master(uint8 remarkId,uint8 type,uint8 *data,int len);
void ICACHE_FLASH_ATTR uart_recv_callback(uint8_t *data,int len);
void ICACHE_FLASH_ATTR uart_recv_passcheck();


int ICACHE_FLASH_ATTR safe_config_refresh_set(uint8 status);
int ICACHE_FLASH_ATTR safe_config_refresh_get();
char* ICACHE_FLASH_ATTR safe_config_read();

int ICACHE_FLASH_ATTR door_config_refresh_set(uint8 status);
int ICACHE_FLASH_ATTR door_config_refresh_get();
char* ICACHE_FLASH_ATTR door_config_read();

int ICACHE_FLASH_ATTR door_others_remarkid_isExist(uint8 remarkId);
char* ICACHE_FLASH_ATTR door_others_read(uint8 remarkId);
int ICACHE_FLASH_ATTR door_others_getlength(uint8 remarkId);



#endif /* DOOR_UART_H_ */
