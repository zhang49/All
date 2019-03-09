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


void ICACHE_FLASH_ATTR door_init();
void ICACHE_FLASH_ATTR request_safe_config();
void ICACHE_FLASH_ATTR request_door_config();
void ICACHE_FLASH_ATTR door_request_all_config();
int ICACHE_FLASH_ATTR door_config_refresh_set(uint8 status);
int ICACHE_FLASH_ATTR safe_config_isrefresh();
int ICACHE_FLASH_ATTR door_config_isrefresh();
char* ICACHE_FLASH_ATTR get_safe_config();
char* ICACHE_FLASH_ATTR get_door_config();


int ICACHE_FLASH_ATTR door_config_write_state();
char* ICACHE_FLASH_ATTR get_door_config_write_ret();
void ICACHE_FLASH_ATTR set_door_config_write_state(uint8 state);

int ICACHE_FLASH_ATTR get_command_write_state();
void ICACHE_FLASH_ATTR set_command_write_state(uint8 state);
int ICACHE_FLASH_ATTR door_databuf_remarkid_isExist(uint8 remarkId);
char* ICACHE_FLASH_ATTR door_others_read(uint8 remarkId);
int ICACHE_FLASH_ATTR door_others_getlength(uint8 remarkId);

#endif /* DOOR_H_ */
