/*
 * comm_sensor.h
 *
 *  Created on: Mar 17, 2019
 *      Author: root
 */

#ifndef COMM_SENSOR_H_
#define COMM_SENSOR_H_

typedef struct {
	uint16 ray_value;
	int temp;
	int hum;
}SensorUnio;


uint32 ICACHE_FLASH_ATTR comm_ray_value_api_get();
uint16 comm_ray_value_espnow_read();
void motor_pas_api(u8 pre);
void motor_pos_api(u8 pre);

void comm_dht22_espnow_read();
u16  ICACHE_FLASH_ATTR comm_temperature_value_read_api();
u16  ICACHE_FLASH_ATTR comm_humidity_value_read_api();

void ICACHE_FLASH_ATTR comm_temperature_value_write_api(int temp);
void ICACHE_FLASH_ATTR comm_humidity_value_write_api(int hum);


#endif /* COMM_SENSOR_H_ */
