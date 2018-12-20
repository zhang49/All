#include "sys.h"
#include <jansson.h>

#ifndef _ESP8266_H
#define _ESP8266_H

#define SSID "zy_em"
#define PASSWORD "12345678"

#define TCP_ADDRESS "192.168.20.103"
#define TCP_PORT 8888

extern u8 sAcceptCount;
extern u8 SERVER_ACCEPT_MAX;
enum ESP8266STARTMODE{
	LOCAL,
	CLOUD
};


void ESP8266_Init(void);
void ESP8266_test(void);
u8 ESP8266_Start(enum ESP8266STARTMODE mode);
u8 ESP8266_LocalMode(u32 port);
u8 ESP8266_CLODEMode(char *type ,char *address, int port);
u8 GetJsonType(json_t *injson,char *type);
u8 getJsonData(json_t *injson,char *param,int *retInteger,char *retCharArray);
u8 getJsonDataCharArray(json_t *injson,char *param,char *retdata);
u8 getJsonDataInteger(json_t *injson,char *param,int retdata);
u8 Send_Json(u8 cid);
void ESP8266_RecvProcess(enum ESP8266STARTMODE mode);
#endif

