/*
 * comm_device.h
 *
 *  Created on: Mar 26, 2019
 *      Author: root
 */

#ifndef COMM_DEVICE_H_
#define COMM_DEVICE_H_

#include "c_types.h"

#define DHT22_CLOSE
#define RELAY1_CLOSE
#define RELAY2_CLOSE
#define RELAY3_CLOSE
#define LIGHT_CLOSE
#define RAY_OPEN

#define RELAY_PIN GPIO_ID_PIN(0)

#define MOTOR_PIN1 GPIO_ID_PIN(5)
#define MOTOR_PIN2 GPIO_ID_PIN(4)
#define MOTOR_PIN3 GPIO_ID_PIN(0)
#define MOTOR_PIN4 GPIO_ID_PIN(14)

typedef enum{
	Motor_Stop,
	Motor_Pos,
	Motor_Pas,
}motor_turn_status;


void comm_device_init();
void ICACHE_FLASH_ATTR comm_relay_init();
void ICACHE_FLASH_ATTR comm_relay_status_set(u8 level);

void dht22_humidity_read_api(uint8_t *data);
void dht22_temperature_read_api(uint8_t *data);

uint16_t ray_read_api();

void ICACHE_FLASH_ATTR motor_duration_set(uint16_t time);
void ICACHE_FLASH_ATTR motor_speed_set(int speed);
void ICACHE_FLASH_ATTR motor_direction_set(motor_turn_status direction);
void ICACHE_FLASH_ATTR motor_move_start(uint8 speed,uint16 duration,motor_turn_status direction);
void motor_set_pin_hight(int pin);

#endif /* COMM_DEVICE_H_ */
