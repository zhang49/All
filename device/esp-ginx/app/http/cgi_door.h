/*
 *
 *
 *
 * */

#ifndef CGI_DOOR_H
#define CGI_DOOR_H

#define NEEDTIMER 1
#define TIMER_SINGLETIME 100
#define TIMER_TIMEROUT 20

int ICACHE_FLASH_ATTR http_door_operator(http_connection *c);

int ICACHE_FLASH_ATTR http_wifi_config_api_read(http_connection *c);
int ICACHE_FLASH_ATTR http_wifi_config_api_write(http_connection *c)
;
int ICACHE_FLASH_ATTR http_safe_config_api_read(http_connection *c);
int ICACHE_FLASH_ATTR http_door_config_api_read(http_connection *c);

int ICACHE_FLASH_ATTR http_door_expect_ret(http_connection *c);


#endif
