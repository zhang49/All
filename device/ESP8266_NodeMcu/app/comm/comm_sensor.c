/*
 * comm_sensor.c
 *
 *  Created on: Mar 17, 2019
 *      Author: root
 */


#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "gpio.h"
#include "comm/comm_sensor.h"
#include "user_config.h"

uint32 ICACHE_FLASH_ATTR comm_ray_value_api_get(){
	return comm_seneors.ray_value;
}
uint32 comm_sensor_ray_value_read(){
	comm_seneors.ray_value=system_adc_read();
	os_printf("ray value is:%d\r\n",comm_seneors.ray_value);
}
