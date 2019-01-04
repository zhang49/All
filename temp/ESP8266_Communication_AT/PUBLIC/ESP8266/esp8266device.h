#include "sys.h"
#ifndef _esp8266device_H
#define _esp8266device_H

#define SERVER_ACCEPT_MAX 3
#define NET_HEAD_SIZE 2
//#define SCR_SLOT_MAX 3


enum CWMODE{
	STATION=1,
	AP,
	BOTH
};

struct SERVER_CLIENT_RECVBUF {
	char rData[512];
	u16 singleLen;
	u16 cursor;
};
extern struct SERVER_CLIENT_RECVBUF sRecvBuf[SERVER_ACCEPT_MAX];
extern u8 sAcceptCount;

extern u8 clientId;
void ESP8266_DEVICE_Init(int baudRate);
u8 ESP8266_CloseEcho(void);
u8 ESP8266_ATTest(u16 tenMsTimes);
u8 ESP8266_SendCmdWithCheck(char *data,char *ret1,char *ret2,u16 tenMsTimes);
u8 ESP8266_SendCmd(char *data);
u8 ESP8266_RST(void);
u8 ESP8266_CmdIsSuccess(char *expect,u16 tenMsTimes);
u8 ESP8266_ReadATRet(char data[],u16 tenMsTimes);
u8 ESP8266_ExpectRet(char *para1,char *para2,u16 tenMsTimes);
u8 ESP8266_CWMODE_Choice(enum CWMODE);
u8 ESP8266_RestoreAPConfig();
u8 ESP8266_SetAPModeConfig(char *ssid,char *psw,int chl,int ecn);
u8 ESP8266_JoinAP(char *ssid,char *psw);
u8 ESP8266_ConnectToServer(char *type,char *ipaddress,int port);
u8 ESP8266_CreateTcpServer(u32 tcpServerPort);
u8 ESP8266_ReadCloudCustom(char data[],u16 timeout);
u8 ESP8266_ReadIpdData(u8 *id);
u8 ESP8266_ReadLocalData(u8 *id,u16 timeout);
void ESP8266_SendNet_unTransparent(u8 cid,const char *ostream);
void ESP8266_printf (char * Data, ... );
void ESP8266_Sendu8(u8 ch);
void ESP8266_senditoa(u32 d);
#endif

