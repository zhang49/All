/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "platform.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "flash_api.h"

#include "osapi.h"

#include "user_interface.h"
#include "user_config.h"

#include "ets_sys.h"
#include "driver/uart.h"
#include "driver/relay.h"
#include "mem.h"

#include "comm_espnow.h"
#include "comm_device.h"
#include "sensor/sensors.h"
	
#define AP_SSID "ESP8266_"DEVICE_NAME
#define AP_PWD ""
#define STA_SSID "Ares"
#define STA_PWD "460204415"

#ifdef DEVELOP_VERSION

#endif





/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{   
    
    system_update_cpu_freq(160); //overclock :)
    uart_init(BIT_RATE_115200,BIT_RATE_115200);
    NODE_DBG("User Init");
	wifi_station_disconnect();
	wifi_station_set_auto_connect(0);

    uint32_t size = flash_get_size_byte();
    NODE_DBG("Flash size %d",size);
    user_esp_now_set_mac_current();
    comm_espnow_init();
    comm_device_init();
    //init_dns();
    //init_http_server();
    //mqtt_app_init();



    #ifdef DEVELOP_VERSION

    //arm timer


    #endif
}
