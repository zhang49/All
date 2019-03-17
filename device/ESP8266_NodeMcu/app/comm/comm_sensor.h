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
	uint16 temptdsdsdsdasdas;
}SensorUnio;
SensorUnio comm_seneors;
uint32 ICACHE_FLASH_ATTR comm_ray_value_api_get();
uint32 comm_sensor_ray_value_read();

#endif /* COMM_SENSOR_H_ */
