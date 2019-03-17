/*
 * comm_light.h
 *
 *  Created on: Mar 17, 2019
 *      Author: root
 */

#ifndef COMM_LIGHT_H_
#define COMM_LIGHT_H_
//D2 GPIO4
#define ALARM_NORMAL_LIGHT_GPIO_ID GPIO_ID_PIN(4)

void ICACHE_FLASH_ATTR light_alarm_open();
void ICACHE_FLASH_ATTR light_alarm_close();
void ICACHE_FLASH_ATTR comm_light_init();


#endif /* COMM_LIGHT_H_ */
