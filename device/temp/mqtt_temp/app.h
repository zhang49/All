#ifndef __MQTT_APP_H
#define __MQTT_APP_H

#define MQTT_SERVER_IP "47.110.254.50"
#define MQTT_SERVER_PORT 9883
#define MQTT_USER_NAME "autodoor_client"
#define MQTT_PASSWORD "autodoor_client123!@#098567pwd"

#define SERIAL_NUMBER "client123"

#define STATUS_PUB_TOPIC "autodoor/status/"SERIAL_NUMBER

void ICACHE_FLASH_ATTR mqtt_app_init();


#endif
