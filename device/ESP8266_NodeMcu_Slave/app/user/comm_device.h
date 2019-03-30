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
#define RELAY1_OPEN
#define RELAY2_CLOSE
#define RELAY3_CLOSE

#define RELAY_PIN GPIO_ID_PIN(0)

void comm_device_init();
void ICACHE_FLASH_ATTR comm_relay_init();
void ICACHE_FLASH_ATTR comm_relay_status_set(u8 level);

void dht22_humidity_read_api(uint8_t *data);
void dht22_temperature_read_api(uint8_t *data);

#endif /* COMM_DEVICE_H_ */
