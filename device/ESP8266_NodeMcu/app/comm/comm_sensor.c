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
#include "espnow.h"
#include "comm/comm_pub_def.h"
#include "comm/comm_espnow.h"


SensorUnio comm_sensors;
u8 DHT22MacAddr[6]  = {0xA2, 0x11, 0xA6, 0x55, 0x55, 0x55};

void ICACHE_FLASH_ATTR comm_sensor_init(){
	esp_now_add_peer(DHT22MacAddr, ESP_NOW_ROLE_SLAVE, EspNowChannel, NULL, 0);

}
uint32 ICACHE_FLASH_ATTR comm_ray_value_api_get(){
	return comm_sensors.ray_value;
}

uint32 comm_sensor_ray_value_read(){
	comm_sensors.ray_value=system_adc_read();
	//os_printf("ray value is:%d\r\n",comm_seneors.ray_value);
}
void comm_dht22_espnow_read(){
	esp_now_send_api(DHT22MacAddr,RequestDht22,NULL);
}
u16 ICACHE_FLASH_ATTR comm_temperature_value_read_api(){
	return comm_sensors.temp;
}
u16 ICACHE_FLASH_ATTR comm_humidity_value_read_api(){
	return comm_sensors.hum;
}
void ICACHE_FLASH_ATTR comm_temperature_value_write_api(int temp){
	comm_sensors.temp = temp;
}
void ICACHE_FLASH_ATTR comm_humidity_value_write_api(int hum){
	comm_sensors.hum = hum;
}
















