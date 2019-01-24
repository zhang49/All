#include "sys.h"
#include <jansson.h>

#define UID_BASE_ADDER 0x1FFFF7E8

#ifndef _ESP8266_H
#define _ESP8266_H


void ESP8266_Init(int baudRate,u32 sendInterval);
void ESP8266_printf (char * Data, ... );
void ESP8266_Sendu8(u8 ch);
void ESP8266_senditoa(u32 d);
u8 ESP8266_ReadNetData(char data[],u16 timeout);
u8 ESP8266_NeedSendData(void);
void ESP8266_SendNetData(char *ostream,u16 len);

void ESP8266_test(void);
u8 GetJsonType(json_t *injson,char *type);
u8 json_find(json_t *injson,char *key,char *ret);
u8 json_add(json_t *injson,char *key,char *value);
u8 ESP8266_Restore();
//u8 getJsonData(json_t *injson,char *param,int *retInteger,char *retCharArray);
//u8 getJsonDataCharArray(json_t *injson,char *param,char *retdata);
//u8 getJsonDataInteger(json_t *injson,char *param,int retdata);

u8 ESP8266_RecvProcess(void);


#endif

