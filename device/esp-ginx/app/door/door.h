/*
 * door.h
 *
 *  Created on: Feb 26, 2019
 *      Author: root
 */

#ifndef DOOR_H_
#define DOOR_H_

#include <sys/types.h>
#include "c_stdint.h"


#include "mqtt/mqtt.h"
#include "http/http.h"

#define NEEDTIMER 1
#define TIMER_SINGLETIME 100
#define TIMER_TIMEROUT 20

#define CLIENT_IS_HTTP 1
#define CLIENT_IS_MQTT 2



typedef struct {
	ETSTimer timer;
	uint8_t remark_id;
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
} mqtt_door_comm_data;

void ICACHE_FLASH_ATTR door_init();

int ICACHE_FLASH_ATTR door_operator_api(void *);
int ICACHE_FLASH_ATTR http_door_operator_api(http_connection *);
int ICACHE_FLASH_ATTR send_to_client(void *client,char *message);


int ICACHE_FLASH_ATTR door_wifi_config_read(void *client);
int ICACHE_FLASH_ATTR door_safe_config_read(void *client);
int ICACHE_FLASH_ATTR door_door_config_read(void *client);
int ICACHE_FLASH_ATTR door_wifi_config_write(void *client);
int ICACHE_FLASH_ATTR door_door_expect_ret(void *client);

void ICACHE_FLASH_ATTR request_master_safe_config();
void ICACHE_FLASH_ATTR request_master_door_config();
void ICACHE_FLASH_ATTR door_request_all_config();


#endif /* DOOR_H_ */
