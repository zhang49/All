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

int ICACHE_FLASH_ATTR comm_wifi_api_get_status(http_connection *c);
void ICACHE_FLASH_ATTR comm_wifi_api_scan_start();
char *ICACHE_FLASH_ATTR comm_wifi_api_scan();
int ICACHE_FLASH_ATTR comm_wifi_api_connect_ap(http_connection *c);
int ICACHE_FLASH_ATTR comm_wifi_api_disconnect(http_connection *c);
int ICACHE_FLASH_ATTR comm_wifi_api_check_internet(http_connection *c);

int ICACHE_FLASH_ATTR comm_wifi_safe_read(void *client);
int ICACHE_FLASH_ATTR comm_wifi_config_read(void *client);
int ICACHE_FLASH_ATTR comm_wifi_config_write(void *client);
int ICACHE_FLASH_ATTR comm_wifi_connect_ap(char *ssid,char *password);
#endif
