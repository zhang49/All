
#ifndef COMM_ESPNOW_H_
#define COMM_ESPNOW_H_
//D2 GPIO4

#ifndef ESPNOW_DBG
#define ESPNOW_DBG NODE_DBG
#else
#define ESPNOW_DBG
#endif

#define ESP_NOW_CONTROLLER

#define ESPNOWSEDNBUFSIZE 4
typedef enum{
	ENSS_Success,
	ENSS_Waitting,
	ENSS_Sending,
	ENSS_Failed
}EspNowSendState;

typedef struct {
	u8 slave_mac[6];
	u8 serial_nb;
	u8 type;
	u8 data[2];
	EspNowSendState status;
}EspNowSendBuf;

void comm_espnow_init();
void ICACHE_FLASH_ATTR user_esp_now_set_mac_current(void);
void comm_esp_now_recv_cb(u8 *mac_addr, u8 *data, u8 len);
void comm_esp_now_send_cb(u8 *mac_addr, u8 status);


#endif /* COMM_LIGHT_H_ */
