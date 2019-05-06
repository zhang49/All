#ifndef __MQTT_APP_H
#define __MQTT_APP_H

#define STATUS_PUB_TOPIC "autodoor/status/"SERIAL_NUMBER


void ICACHE_FLASH_ATTR mqtt_publish_api(const char* topic, const char* data, int data_length, int qos, int retain);

int ICACHE_FLASH_ATTR mqtt_is_connected();

void ICACHE_FLASH_ATTR mqtt_app_init();
#endif
