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

#include "comm_uart.h"
#include "comm_espnow.h"
#include "comm_pub_def.h"
#include "espnow.h"

os_timer_t esp_now_timer;
uint8 sendSerialNb;
u8 ControllerMacAddr[6] = {0xA0, 0x20, 0xA6, 0xAA, 0xAA, 0xAA};

u8 SalveMacAddrVector[ESPNOWSLAVEMAX][6] = {
		{0xA2, 0x01, 0xA6, 0x55, 0x55, 0x55},
		{0xA2, 0x02, 0xA6, 0x55, 0x55, 0x55},
		{0xA2, 0x03, 0xA6, 0x55, 0x55, 0x55},
		{0xA2, 0x04, 0xA6, 0x55, 0x55, 0x55},
		{0xA2, 0x05, 0xA6, 0x55, 0x55, 0x55},
		{0xA2, 0x06, 0xA6, 0x55, 0x55, 0x55},
};


EspNowSendBuf esp_now_send_buf[ESPNOWSEDNBUFSIZE];

typedef struct {
	u8 xorCheck;
	u8 type;
	u8 sNumber;
	u8 data[5];
	u8 cursor;
	u8 mac_addr[6];
}EspNowRecvBuf;
EspNowRecvBuf esp_now_recv_buf;

typedef enum  {
	ENRF_Head1,
	ENRF_Head2,
	ENRF_SNumber,
	ENRF_Type,
	ENRF_Data,
	ENRF_XorCheck
}EspNowRecvFlag;
EspNowRecvFlag eNRF;

int last_serial_bumber=-1;
u8 send_tmr_tick=0;

void DisplayMac(char* msg,u8 *mac){
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
void comm_esp_now_recv_cb(u8 *mac_addr, u8 *data, u8 len){
	u8 bytes[30];
	os_memcpy(bytes,data,len+1);
	u8 i=0;
	char str[50]={0};
	for(i=0;i<len;i++){
		os_sprintf(str,(i==0?"%s%2x":"%s_%2x"),str,bytes[i]);
	}
	static int recvcount=0;
	os_printf("\t\t%d.esp now recv\r\n",recvcount++);
	for(i=0;i<len;i++){
restart:
		if(eNRF!=ENRF_XorCheck)
			esp_now_recv_buf.xorCheck^=bytes[i];
		switch(eNRF){
			case ENRF_Head1:
				if(bytes[i]==0xaa){
					eNRF=ENRF_Head2;
					esp_now_recv_buf.xorCheck=bytes[i];
				}else eNRF=ENRF_Head1;
				break;
			case ENRF_Head2:
				if(bytes[i]==0xbb){
					eNRF=ENRF_SNumber;
				}else {
					eNRF=ENRF_Head1;
					goto restart;
				}
				break;
			case ENRF_SNumber:
				eNRF=ENRF_Type;
				esp_now_recv_buf.sNumber=bytes[i];
				break;
			case ENRF_Type:
				eNRF=ENRF_Data;
				esp_now_recv_buf.type=bytes[i];
				esp_now_recv_buf.cursor=0;
				break;
			case ENRF_Data:
				if(esp_now_recv_buf.cursor<5){
					esp_now_recv_buf.data[esp_now_recv_buf.cursor]=bytes[i];
					esp_now_recv_buf.cursor++;
					if(esp_now_recv_buf.cursor==5)
						eNRF=ENRF_XorCheck;
				}
				break;
			case ENRF_XorCheck:
				eNRF=ENRF_Head1;
				if(esp_now_recv_buf.xorCheck==bytes[i]){
					//pass	,data in byte 2 and 3
					if(esp_now_recv_buf.sNumber!=last_serial_bumber){
						os_printf("ESPNOW: Recv type:%3x, data %3x%3x\r\n",esp_now_recv_buf.type,
								esp_now_recv_buf.data[0],esp_now_recv_buf.data[1]);
						last_serial_bumber=esp_now_recv_buf.sNumber;
						os_memcpy(esp_now_recv_buf.mac_addr,mac_addr,6);
						comm_esp_now_recv_data();
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
}



void comm_esp_now_sendMsg(EspNowSendBuf sbuf){
	u8 ostream[10]={0};
	u8 index=0,i;
	ostream[index++]=0xaa;
	ostream[index++]=0xbb;
	ostream[index++]=sbuf.serial_nb;
	ostream[index++]=sbuf.type;
	//
	for(i=0;i<5;i++){
		ostream[index++]=sbuf.data[i];
	}
	ostream[9]=0;
	for(i=0;i<9;i++)ostream[9]^=ostream[i];
	esp_now_send(sbuf.slave_mac, ostream, 10);
	if(sbuf.failed_count==0)
		DisplayMac("esp now send msg to",sbuf.slave_mac);
}


int ICACHE_FLASH_ATTR esp_now_send_api(uint8_t *mac,uint8_t type,uint8_t *data){
	int i;
	for(i=0;i<ESPNOWSEDNBUFSIZE;i++){
		if(esp_now_send_buf[i].status!=ENSS_Success && os_memcmp(esp_now_send_buf[i].slave_mac,mac,6)==0){
			DisplayMac("esp_now_send_api has this mac",mac);
			return -1;
		}
	}
	for(i=0;i<ESPNOWSEDNBUFSIZE;i++){
		if(esp_now_send_buf[i].status==ENSS_Success){
			esp_now_send_buf[i].status=ENSS_Waitting;
			os_memcpy(esp_now_send_buf[i].slave_mac,mac,6);
			esp_now_send_buf[i].type=type;
			esp_now_send_buf[i].serial_nb=sendSerialNb++;
			esp_now_send_buf[i].failed_count = 0;
			if(data==NULL){
				os_memset(esp_now_send_buf[i].data,0,5);
			}else{
				os_memcpy(esp_now_send_buf[i].data,data,5);
			}
			DisplayMac("esp_now_send_api send",mac);
			return 1;
		}
	}
	DisplayMac("esp_now_send_api failed return -1",mac);
	return -1;
}
//buf send failed, send again;

void comm_esp_now_send_cb(u8 *mac_addr, u8 status){
	u8 i;
	for(i=0;i<ESPNOWSEDNBUFSIZE;i++){
		//DisplayMac("buf mac:",slave_mac);
		//DisplayMac("cb mac:",mac_addr);
		if(os_memcmp(esp_now_send_buf[i].slave_mac,mac_addr,6)==0){
			if(status==0){//send success
				os_memset(esp_now_send_buf[i].slave_mac,0,6);
				esp_now_send_buf[i].status=ENSS_Success;
				DisplayMac("espnow send success to ",mac_addr);
			}else{
				esp_now_send_buf[i].status=ENSS_Failed;
				esp_now_send_buf[i].failed_count++;
				if(esp_now_send_buf[i].failed_count==ESPNOWSENDERRORMAX){
					esp_now_send_buf[i].status=ENSS_Success;
					DisplayMac("espnow send failed to ",mac_addr);
				}
			}
		}
	}
}
static void comm_esp_now_send_tmr(){
	u8 i;
	for(i=0;i<ESPNOWSEDNBUFSIZE;i++){
		if(esp_now_send_buf[i].status==ENSS_Waitting){
			esp_now_send_buf[i].status=ENSS_Sending;
			comm_esp_now_sendMsg(esp_now_send_buf[i]);
		}
		 if(esp_now_send_buf[i].status==ENSS_Failed){
				esp_now_send_buf[i].status=ENSS_Sending;
			comm_esp_now_sendMsg(esp_now_send_buf[i]);
		 }
	}
}

void comm_esp_now_recv_data(){
	os_printf("send_message_to_master\r\n");
	int mac_index = esp_now_recv_buf.mac_addr[1]-1;
	send_message_to_master(mac_index,esp_now_recv_buf.type,
			esp_now_recv_buf.data);
}

void ICACHE_FLASH_ATTR user_esp_now_set_mac_current(void)
{
    // 设置station MAC地址
    wifi_set_macaddr(STATION_IF, ControllerMacAddr);
    // 设置为station模式
    wifi_set_opmode_current(STATIONAP_MODE);
	os_printf("controller user_esp_now_set_mac_current over.\r\n");
}

char *ICACHE_FLASH_ATTR getMacAddrByIndex(uint8_t index){
	return SalveMacAddrVector[index];
}
void ICACHE_FLASH_ATTR comm_espnow_init(){
	if (esp_now_init() == 0) {
		ESPNOW_DBG("ESPNOW: init successful\n");
		// 注册 ESP-NOW 收包的回调函数
		esp_now_register_recv_cb(comm_esp_now_recv_cb);
		// 注册发包回调函数
		esp_now_register_send_cb(comm_esp_now_send_cb);

	esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
	u8 i=0;
	for(i=0;i<ESPNOWSLAVEMAX;i++){
		esp_now_add_peer(SalveMacAddrVector[i], ESP_NOW_ROLE_SLAVE, EspNowChannel, NULL, 16);
	}
	wifi_station_set_auto_connect(0);
	wifi_station_disconnect();
	os_timer_disarm(&esp_now_timer);
	os_timer_setfn(&esp_now_timer, comm_esp_now_send_tmr, NULL);
	os_timer_arm(&esp_now_timer, 50, 1);
	os_printf("comm_espnow_init over.\r\n");
	}
}















