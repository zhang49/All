/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Israel Lot <me@israellot.com> and Jeroen Domburg <jeroen@spritesmods.com> 
 * wrote this file. As long as you retain this notice you can do whatever you 
 * want with this stuff. If we meet some day, and you think this stuff is 
 * worth it, you can buy us a beer in return. 
 * ----------------------------------------------------------------------------
 */

#ifndef COMM_WIFI_H
#define COMM_WIFI_H

extern int comm_positive;
void 	ICACHE_FLASH_ATTR comm_wifi_connect_default_ap_api();
int 	ICACHE_FLASH_ATTR comm_wifi_connect_ap_check_api();
int 	ICACHE_FLASH_ATTR comm_wifi_start_connect_ap_api(char *ssid,char *password);
int 	ICACHE_FLASH_ATTR comm_wifi_ap_config_write_api(cJSON *root_data);
cJSON * ICACHE_FLASH_ATTR comm_wifi_config_read_api();
cJSON *	ICACHE_FLASH_ATTR comm_wifi_scan_api();
int 	ICACHE_FLASH_ATTR comm_wifi_scan_start_api();
void    ICACHE_FLASH_ATTR wifi_connect_check(int tick_time);
static void ICACHE_FLASH_ATTR comm_wifi_api_scan_callback(void *arg, STATUS status);
#endif
