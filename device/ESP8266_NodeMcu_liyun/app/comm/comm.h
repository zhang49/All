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
#include "json/cJson.h"

#define NEEDTIMER 1
#define COMM_TIMER_SINGLE_TIME 100
#define COMM_TIMER_TIMEOUT 1500

#define CLIENT_IS_HTTP 1
#define CLIENT_IS_MQTT 2

#define DEVICE_TOKEN "17173"

#define GET_LAST_STR(x) get_struct_last_str(#x)


typedef struct {
	ETSTimer timer;
	char *data;
	enum CommMsgType msgtype;
	u32 tickcount;
} session_data_buf;

typedef struct {
	uint8 sign;
	const char topic[120];
	session_data_buf buf;
}mqtt_user_data;

#define ErrorCodeSize 5

enum ErrorCode{
	EC_Normal,
	EC_None,
	EC_Failed,
	EC_Busy,
	EC_Unknown
}ErrorCode;

typedef struct{
	enum ErrorCode error_code;
	char *error_str;
}ErrorMsg;
void ICACHE_FLASH_ATTR comm_init();

int ICACHE_FLASH_ATTR send_to_client(void *client,char *message);

int ICACHE_FLASH_ATTR http_operator_api(http_connection *);
int ICACHE_FLASH_ATTR mqtt_operator_api(char *topic,const char* data,uint32 len);


int ICACHE_FLASH_ATTR comm_connect_syn(void *client);

int ICACHE_FLASH_ATTR comm_wifi_config_read(void *client);
int ICACHE_FLASH_ATTR comm_wifi_ap_config_write(void *client);

int ICACHE_FLASH_ATTR comm_led_pwm_duty_write(void *client);
int ICACHE_FLASH_ATTR comm_ray_motor_cw(void *client);
int ICACHE_FLASH_ATTR comm_ray_motor_ccw(void *client);
int ICACHE_FLASH_ATTR comm_ray_motor_start(void *client);
int ICACHE_FLASH_ATTR comm_ray_motor_stop(void *client);
int ICACHE_FLASH_ATTR comm_ray_value_read(void *client);
int ICACHE_FLASH_ATTR comm_ray_alarm_value_write(void *client);

int ICACHE_FLASH_ATTR comm_expect_ret(void *client);
int ICACHE_FLASH_ATTR comm_syn_status_read(void *client);

mqtt_user_data *ICACHE_FLASH_ATTR create_mqtt_user_data(char *tpoic);
char *ICACHE_FLASH_ATTR replace_str_for_mqtt_transmit(char *destStr,char *headStr);
int ICACHE_FLASH_ATTR mqtt_send_wifiscan();

const char *ICACHE_FLASH_ATTR get_res_type(void *client);
uint8 ICACHE_FLASH_ATTR http_send_string(void *client,char *str);
uint8 ICACHE_FLASH_ATTR mqtt_send_string(void *client,char *str);
uint8 ICACHE_FLASH_ATTR send_ret_json(void *client,cJSON *retroot,enum ErrorCode error_code);
char *ICACHE_FLASH_ATTR get_struct_last_str(char *str);

#endif /* DOOR_H_ */
