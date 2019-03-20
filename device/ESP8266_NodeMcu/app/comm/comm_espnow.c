/*
 * comm_espnow.c
 *
 *  Created on: Mar 19, 2019
 *      Author: root
 */

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_config.h"

#include "comm/comm_espnow.h"
#include "espnow.h"


os_timer_t esp_now_timer;
//station mac, slave role mac
//slave sta mac 5C-CF-7F-C2-56-3D
//master sta mac 86-0D-8E-96-5D-7A
uint8 slave_mac[6]={0x5c,0xcf,0x7f,0xc2,0x56,0x3d};
uint8 master_mac[6]={0x86,0x0d,0x8e,0x96,0x5d,0x7a};
uint8 isMaster=0;

void comm_esp_now_recv_cb(u8 *mac_addr, u8 *data, u8 len){
	char *dd=(char *)os_zalloc(len);
	os_memcpy(dd,data,len+1);
	ESPNOW_DBG("ESPNOW: recv data to %s",dd);
}

void comm_esp_now_send_cb(u8 *mac_addr, u8 status){

}

static void comm_esp_now_send_test(){
	char *send_data="esp_now test!";
	esp_now_send(slave_mac, send_data, os_strlen(send_data));
	int i=0;
	char mac_str[13];
	for(i=0;i<6;i++){
		os_sprintf(mac_str,"%2x",slave_mac[i]);
	}
	ESPNOW_DBG("ESPNOW: send data to %s",mac_str);
}

void comm_espnow_init(){
	if (esp_now_init() == 0) {
		ESPNOW_DBG("ESPNOW: init successful\n");
		// 注册 ESP-NOW 收包的回调函数
		esp_now_register_recv_cb(comm_esp_now_recv_cb);
		// 注册发包回调函数
		esp_now_register_send_cb(comm_esp_now_send_cb);
	}
	if(isMaster){
		esp_now_set_self_role(ESP_NOW_ROLE_MAX);
		esp_now_add_peer(slave_mac, ESP_NOW_ROLE_MAX, 1, NULL, 16);
		os_timer_disarm(&esp_now_timer);
		os_timer_setfn(&esp_now_timer, comm_esp_now_send_test, NULL);
		os_timer_arm(&esp_now_timer, 1000, 1);
	}else{
		esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
		esp_now_add_peer(master_mac, ESP_NOW_ROLE_SLAVE, 1, NULL, 16);
	}
}















