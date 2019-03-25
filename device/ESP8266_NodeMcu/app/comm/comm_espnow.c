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
#include "mem.h"

#include "comm/comm_espnow.h"
#include "espnow.h"

os_timer_t esp_now_timer;
uint8 sendSerialNb;
//my esp8266 12F ap macaddr 86-0D-8E-96-5D-7A
//uint8 slave_mac[6]={0x86,0x0d,0x8E,0x96,0x5d,0x7a};
uint8 controller_mac[6]={0x02,0x02,0x03,0x04,0x05,0x06};
u8 slave_mac[6]={0xfe,0x02,0x03,0x04,0x05,0x06};

//uint8 slave_mac[6]={0x5e,0xcf,0x7f,0xc2,0x56,0x3d};


EspNowSendBuf esp_now_send_buf[ESPNOWSEDNBUFSIZE];
typedef struct {
	u8 totalCheck;
	u8 type;
	u8 sNumber;
	u8 data[2];
}EspNowRecvBuf;
EspNowRecvBuf esp_now_recv_buf;
typedef enum  {
	Head1,
	Head2,
	SNumber,
	Type,
	Data1,
	Data2,
	TotalCheck
}CommandFlag;
CommandFlag CF;

void DispalyMac(char* msg,u8 *mac){
	int i;
	char mac_str[50];
	char temp[10];
	os_memset(mac_str,0,20);
	for(i=0;i<6;i++){
		os_sprintf(temp,i==0?"%2x":"-%2x",mac[i]);
		os_strcat(mac_str,temp);
	}
	ESPNOW_DBG("%s %s",msg,mac_str);
}
int last_serial_bumber=-1;
void comm_esp_now_recv_cb(u8 *mac_addr, u8 *data, u8 len){
	u8 *bytes=(u8 *)os_zalloc(len);
	os_memcpy(bytes,data,len+1);
	u8 i=0;
	for(i=0;i<len;i++){
		os_printf("%3x-",bytes[i]);
	}
	ESPNOW_DBG("");
	for(i=0;i<len;i++){
restart:
		if(CF!=TotalCheck)
			esp_now_recv_buf.totalCheck^=bytes[i];
		switch(CF){
			case Head1:
				if(bytes[i]==0xaa){
					CF=Head2;
					esp_now_recv_buf.totalCheck=bytes[i];
				}else CF=Head1;
				break;
			case Head2:
				if(bytes[i]==0xbb){
					CF=SNumber;
				}else {
					CF=Head1;
					goto restart;
				}
				break;
			case SNumber:
				CF=Type;
				esp_now_recv_buf.sNumber=bytes[i];
				break;
			case Type:
				CF=Data1;
				esp_now_recv_buf.type=bytes[i];
				break;
			case Data1:
				CF=Data2;
				esp_now_recv_buf.data[0]=bytes[i];
				break;
			case Data2:
				CF=TotalCheck;
				esp_now_recv_buf.data[1]=bytes[i];
				break;
			case TotalCheck:
				CF=Head1;
				if(esp_now_recv_buf.totalCheck==bytes[i]){
					//pass	,data in byte 2 and 3
					if(esp_now_recv_buf.sNumber!=last_serial_bumber){
						ESPNOW_DBG("ESPNOW: Recv type:%3x, data %3x%3x",esp_now_recv_buf.type,
								esp_now_recv_buf.data[0],esp_now_recv_buf.data[1]);
						last_serial_bumber=esp_now_recv_buf.sNumber;
					}else{
						ESPNOW_DBG("ESPNOW: Recv the same serial Number.");
					}
				}else{
					ESPNOW_DBG("ESPNOW: Don't pass check");
					goto restart;
				}
				break;
		}
	}
	os_free(bytes);
}

void comm_esp_now_sendMsg(EspNowSendBuf sbuf){
	u8 ostream[7]={0};
	ostream[0]=0xaa;
	ostream[1]=0xbb;
	ostream[2]=sbuf.serial_nb;
	ostream[3]=sbuf.type;
	//
	ostream[4]=0x00;
	ostream[5]=0x00;
	int i=0;
	ostream[6]=ostream[0];
	for(i=1;i<6;i++)ostream[6]^=ostream[i];
	esp_now_send(sbuf.slave_mac, ostream, 7);
	DispalyMac("send msg to",sbuf.slave_mac);
}


int esp_now_send_api(uint8_t *mac,uint8_t type,uint8_t data[2]){
	int i;
	for(i=0;i<ESPNOWSEDNBUFSIZE;i++){
		if(esp_now_send_buf[i].status!=ENSS_Success && os_memcmp(esp_now_send_buf[i].slave_mac,mac,6)!=0){
			return -1;
		}
	}
	for(i=0;i<ESPNOWSEDNBUFSIZE;i++){
		if(esp_now_send_buf[i].status==ENSS_Success){
			esp_now_send_buf[i].status=ENSS_Waitting;
			os_memcpy(esp_now_send_buf[i].slave_mac,mac,6);
			esp_now_send_buf[i].type=type;
			os_memcpy(esp_now_send_buf[i].data,data,2);
			ESPNOW_DBG("api send");
			return 1;
		}else{
			return -1;
		}
	}
}
//buf send failed, send again;

void comm_esp_now_send_cb(u8 *mac_addr, u8 status){
	u8 i;
	ESPNOW_DBG("Send callback.");
	for(i=0;i<ESPNOWSEDNBUFSIZE;i++){
		//DispalyMac("buf mac:",slave_mac);
		//DispalyMac("cb mac:",mac_addr);
		if(os_memcmp(esp_now_send_buf[i].slave_mac,mac_addr,6)==0){
			if(status==0){//send success
				sendSerialNb++;
				os_memset(esp_now_send_buf[i].slave_mac,0,6);
				esp_now_send_buf[i].status=ENSS_Success;
				ESPNOW_DBG("Send success.");
			}else{
				esp_now_send_buf[i].status=ENSS_Failed;
				ESPNOW_DBG("sedn failed.");
			}
		}
	}
}
u8 send_tmr_tick=0;
static void comm_esp_now_send_tmr(){
	u8 i;
	for(i=0;i<ESPNOWSEDNBUFSIZE;i++){
		if(esp_now_send_buf[i].status==ENSS_Waitting){
			esp_now_send_buf[i].serial_nb=sendSerialNb;
			esp_now_send_buf[i].status=ENSS_Sending;
			comm_esp_now_sendMsg(esp_now_send_buf[i]);
		}
		 if(esp_now_send_buf[i].status==ENSS_Failed){
			comm_esp_now_sendMsg(esp_now_send_buf[i]);
		 }
	}
	send_tmr_tick++;
	if(send_tmr_tick==5){
		send_tmr_tick=0;
		uint8_t data[2]={0};
		data[0]=0xcc;
		data[1]=0xdd;
		esp_now_send_api(slave_mac,0x01,data);
	}
}


void ICACHE_FLASH_ATTR user_esp_now_set_mac_current(void)
{
#if defined(ESP_NOW_SLAVE)
    wifi_set_macaddr(SOFTAP_IF, slave_mac);
    wifi_set_opmode_current(SOFTAP_MODE);
#elif defined(ESP_NOW_CONTROLLER)
    // 设置station MAC地址
    wifi_set_macaddr(STATION_IF, controller_mac);
    // 设置为station模式
    //wifi_set_opmode_current(STATION_MODE);
#endif
}

void comm_espnow_init(){
	if (esp_now_init() == 0) {
		ESPNOW_DBG("ESPNOW: init successful\n");
		// 注册 ESP-NOW 收包的回调函数
		esp_now_register_recv_cb(comm_esp_now_recv_cb);
		// 注册发包回调函数
		esp_now_register_send_cb(comm_esp_now_send_cb);
	}
	esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
	esp_now_add_peer(slave_mac, ESP_NOW_ROLE_SLAVE, 3, NULL, 0);
	os_timer_disarm(&esp_now_timer);
	os_timer_setfn(&esp_now_timer, comm_esp_now_send_tmr, NULL);
	os_timer_arm(&esp_now_timer, 100, 1);
}















