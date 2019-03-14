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

#include "http/http.h"

#define HEADLENGTH 5

#define COMM_NOREFRESH 0
#define COMM_REFRESHING 1
#define COMM_REFRESHED 2

enum UartRecvState{
	URS_FLAGE1,
	URS_FLAGE2,
	URS_LNE_H,
	URS_LNE_L,
	URS_TYPE,
	URS_DATA,
	URS_CHECK
};
enum MasterMsgType{
	SYS_COMMAND = 0x00,		//Such as Mode Restore
	NORMAL_CONFIG = 0x01,		//Such as Mode Restore
	SYN_CONTROL = 0x02,		//synchronization control
	SYN_STATE = 0x03,			//synchronization state
	NORMAL_WIFI_SCAN = 0x99
}master_msg_type;
typedef struct {
	uint8 sm_state;
	uint8 comm_state;
	uint8 temperature;
	uint8 wetness;
	uint8 power;
	uint8 run_time;
}SynState;
SynState syn_state;

enum SynControl{
	INPUT_FLAG_OPEN = 0x08,
	INPUT_FLAG_CLOSE = 0x10,
	INPUT_FLAG_FREEZE = 0x20,
	INPUT_FLAG_UNFREEZE = 0x40
}syn_control;

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
	int refresh_state;//0 :no refresh,1 :refreshing,2 :refresh success
}data_save;
typedef struct {
	data_save config;		//save door_config
	data_save syn_control;		//save syn_control
	uint8 door_config_write_state;
}door_comm_buf;

void ICACHE_FLASH_ATTR door_uart_init();
void ICACHE_FLASH_ATTR send_message_to_master(enum MasterMsgType msg_type,uint8 *data,int len);
void ICACHE_FLASH_ATTR uart_recv_callback(uint8_t *data,int len);
void ICACHE_FLASH_ATTR uart_recv_passcheck();

void ICACHE_FLASH_ATTR door_config_refresh_set(uint8 status);
int ICACHE_FLASH_ATTR door_config_refresh_get();
char* ICACHE_FLASH_ATTR door_config_read();

void ICACHE_FLASH_ATTR door_config_write_status_set(uint8 status);
int ICACHE_FLASH_ATTR door_config_write_status_get();

void ICACHE_FLASH_ATTR syn_control_refresh_set(uint8 status);
int ICACHE_FLASH_ATTR syn_control_refresh_get();
char* ICACHE_FLASH_ATTR syn_control_read();

#endif /* DOOR_UART_H_ */
