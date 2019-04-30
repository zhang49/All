#ifndef __CONFIG_H
#define __CONFIG_H

#include "cpu_esp8266.h"
#include "driver/relay.h"

#define CONFIG_SECTOR (FLASH_SEC_NUM - 6) //last sector
#define CONFIG_ADDRESS (INTERNAL_FLASH_START_ADDRESS+CONFIG_SECTOR*SPI_FLASH_SEC_SIZE)

#define CONFIG_MAGIC 0x66666662



//4-ALIGN EVERYTHING !!

typedef struct {

	uint8_t gpioPin;
	uint8_t state;

	uint8_t pad[2];
} relay;

typedef struct {

	uint32_t magic;//for remark
	uint32_t fill_4byte;//for remark
	uint8_t fillbyte[2];
	uint8_t light_duty;
	uint8_t alarm_ray_value;
} config_data;

void config_save(config_data * data);
config_data * config_read();
int config_init();



#endif
