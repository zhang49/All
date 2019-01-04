#include "sys.h"
#include <jansson.h>

#ifndef _ESP8266_H
#define _ESP8266_H

#define STATIONSSID "zy_em"
#define STATIONPSW "12345678"

#define APSSID "ESP8266_Restore"
#define APPSW "12345678"
#define APTcpServerPort 7641	//AP模式下创建的服务器端口
#define APCHL 5
#define APECN 3
#define CLOUD_TCP_ADDRESS "192.168.20.103"
#define CLOUD_TCP_PORT 7641
#define AT_RESEND_MAX 5		//发送失败时，最大重发次数

extern u8 sAcceptCount;
extern u8 SERVER_ACCEPT_MAX;
enum ESP8266STARTMODE{
	LOCAL,
	CLOUD
};


void ESP8266_Init(void);
void ESP8266_test(void);
u8 ESP8266_Start(enum ESP8266STARTMODE mode,char *ssid,char *psw,int restartflag);
u8 ESP8266_LocalMode(u32 port,char *ssid,char *psw);
u8 ESP8266_CLOUDMode(char *type ,char *ssid,char *psw,char *address, int port);
u8 GetJsonType(json_t *injson,char *type);
u8 json_find(json_t *injson,char *key,char *ret);
u8 json_add(json_t *injson,char *key,char *value);
u8 getJsonData(json_t *injson,char *param,int *retInteger,char *retCharArray);
u8 getJsonDataCharArray(json_t *injson,char *param,char *retdata);
u8 getJsonDataInteger(json_t *injson,char *param,int retdata);
u8 Send_Json(u8 cid);
u8 ESP8266_RecvProcess(enum ESP8266STARTMODE mode);
u8 ESP8266_ReadLocal(void);
#endif

