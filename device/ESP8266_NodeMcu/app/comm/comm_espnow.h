
#ifndef COMM_ESPNOW_H_
#define COMM_ESPNOW_H_
//D2 GPIO4

#ifndef ESPNOW_DBG
#define ESPNOW_DBG NODE_DBG
#else
#define ESPNOW_DBG
#endif



void comm_espnow_init();

void comm_esp_now_recv_cb(u8 *mac_addr, u8 *data, u8 len);
void comm_esp_now_send_cb(u8 *mac_addr, u8 status);


#endif /* COMM_LIGHT_H_ */
