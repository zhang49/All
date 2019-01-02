#include "sys.h"
#ifndef _esp8266device_H
#define _esp8266device_H

#define NET_HEAD_SIZE 2
//#define SCR_SLOT_MAX 3

void ESP8266_DEVICE_Init(int baudRate);

void ESP8266_printf (char * Data, ... );
void ESP8266_Sendu8(u8 ch);
void ESP8266_senditoa(u32 d);

u8 ESP8266_ReadNetData(char data[],u16 timeout);

u8 ESP8266_ReadLocalData(u8 *id,u16 timeout);
void ESP8266_SendNetData(const char *ostream);
#endif

