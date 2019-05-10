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
#include "comm/comm_uart.h"


SensorUnio comm_sensors;

uint8_t ray_alarm_value=35;
void ICACHE_FLASH_ATTR comm_sensor_init(){
	//esp_now_add_peer(DHT22MacAddr, ESP_NOW_ROLE_SLAVE, EspNowChannel, NULL, 0);
	//esp_now_add_peer(RayMacAddr, ESP_NOW_ROLE_SLAVE, EspNowChannel, NULL, 0);
}

uint8_t ICACHE_FLASH_ATTR comm_ray_value_api_get(){
	return comm_sensors.ray_value;
}

void comm_ray_value_write_api(uint8_t value){
	comm_sensors.ray_value = value/255.0*100;
}

void ICACHE_FLASH_ATTR motor_move_espnow_write(int speed,int duration,motor_turn_status direction){
	uint16_t drt=duration&0xffff;
	uint8_t data[5]={speed,drt>>8,drt&0xff,direction};
	send_message_to_slave(RAY_MACINDEX,RequestMotorMove,data);
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
















