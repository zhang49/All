
#ifndef COMM_ESPNOW_H_
#define COMM_ESPNOW_H_
//D2 GPIO4

#ifndef ESPNOW_DBG
#define ESPNOW_DBG NODE_DBG
#else
#define ESPNOW_DBG
#endif

#define ESP_NOW_SLAVE

#define ESPNOWSLAVEMAX 6
#define ESPNOWSEDNBUFSIZE 4

#define ESPNOWSENDERRORMAX 10

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
	u8 data[5];
	u8 failed_count;
	EspNowSendState status;
}EspNowSendBuf;

void ICACHE_FLASH_ATTR comm_espnow_init();
char *ICACHE_FLASH_ATTR getMacAddrByIndex(uint8_t index);

void comm_esp_now_recv_data();
void ICACHE_FLASH_ATTR user_esp_now_set_mac_current(void);
void comm_esp_now_recv_cb(u8 *mac_addr, u8 *data, u8 len);
void comm_esp_now_send_cb(u8 *mac_addr, u8 status);
int ICACHE_FLASH_ATTR esp_now_send_api(uint8_t *mac,uint8_t type,uint8_t *data);

#endif /* COMM_LIGHT_H_ */
