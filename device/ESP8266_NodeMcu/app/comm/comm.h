/*
 * door.h
 *
 *  Created on: Feb 26, 2019
 *      Author: root
 */

#ifndef COMM_H_
#define COMM_H_

#include <sys/types.h>
#include "c_stdint.h"


#include "mqtt/mqtt.h"
#include "http/http.h"
#include "comm/comm_uart.h"

#define NEEDTIMER 1
#define TIMER_SINGLETIME 100
#define TIMER_TIMEROUT 20

#define CLIENT_IS_HTTP 1
#define CLIENT_IS_MQTT 2

#define DEVICE_TOKEN "17173"

#define GET_LAST_STR(x) get_struct_last_str(#x)

typedef struct {
	ETSTimer timer;
	enum MasterMsgType msgtype;
	uint8_t tickcount;
} client_handle_timer;

typedef struct {
	const char *sub_topic;
	uint8 sub_qos;
	const char *msg;
	uint32 data_len;
	const char *pub_topic;
	uint8 pub_qos;
	client_handle_timer timer;
} mqtt_comm_data;

void ICACHE_FLASH_ATTR door_init();

int ICACHE_FLASH_ATTR door_operator_api(void *);
int ICACHE_FLASH_ATTR http_operator_api(http_connection *);
int ICACHE_FLASH_ATTR mqtt_operator_api(MQTT_Client *);
int ICACHE_FLASH_ATTR send_to_client(void *client,char *message);

int ICACHE_FLASH_ATTR door_door_config_read(void *client);

int ICACHE_FLASH_ATTR comm_expect_ret(void *client);
int ICACHE_FLASH_ATTR door_syn_status_read(void *client);

void ICACHE_FLASH_ATTR request_master_safe_config();
void ICACHE_FLASH_ATTR request_master_door_config();



void ICACHE_FLASH_ATTR door_request_all_config();
void ICACHE_FLASH_ATTR request_master_door_config();
char *ICACHE_FLASH_ATTR get_struct_last_str(char *str);

#endif /* DOOR_H_ */
