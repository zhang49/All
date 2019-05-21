/*
 * comm_ligth.c
 *
 *  Created on: Mar 17, 2019
 *      Author: root
 */


#include "ets_sys.h"

#include "osapi.h"
#include "user_interface.h"
#include "gpio.h"

#include "comm_light.h"

#define INT_GPIO	5

void ICACHE_FLASH_ATTR comm_light_init(){

}
void ICACHE_FLASH_ATTR light_alarm_open(){
	GPIO_OUTPUT_SET(ALARM_NORMAL_LIGHT_GPIO_ID, 1);//GPIO4为低
}

void ICACHE_FLASH_ATTR light_alarm_close(){
	GPIO_OUTPUT_SET(ALARM_NORMAL_LIGHT_GPIO_ID, 0);//GPIO4为高
}

