/*
 * comm_sensor.h
 *
 *  Created on: Mar 17, 2019
 *      Author: root
 */

#ifndef COMM_SENSOR_H_
#define COMM_SENSOR_H_

typedef struct {
	uint8 ray_value;	//[0,100]
	int temp;		//1 bit '+' ro '-', data: [0,65535] , 2 decimals
	int hum;		//[0,65535] , 2 decimals noraml 0-100%
}SensorUnio;

typedef enum{
	Motor_Stop,
	Motor_Pos,
	Motor_Pas,
}motor_turn_status;

extern uint8_t ray_alarm_value;

uint8_t ICACHE_FLASH_ATTR comm_ray_value_api_get();
void comm_ray_value_espnow_read();
void comm_ray_value_write_api(uint8_t value);

void comm_ray_config_espnow_write(uint8 ray_alarm_value,uint8 motor_speed);
void ICACHE_FLASH_ATTR motor_move_espnow_write(int speed,int duration,motor_turn_status direction);
u16  ICACHE_FLASH_ATTR comm_temperature_value_read_api();
u16  ICACHE_FLASH_ATTR comm_humidity_value_read_api();

void ICACHE_FLASH_ATTR comm_temperature_value_write_api(int temp);
void ICACHE_FLASH_ATTR comm_humidity_value_write_api(int hum);


#endif /* COMM_SENSOR_H_ */
