#include "sys.h"
#include <jansson.h>
#include "cJSON.h"
#define UID_BASE_ADDER 0x1FFFF7E8

#ifndef _ESP8266_H
#define _ESP8266_H

#define DMA_SINGLE_LEN_MAX 200

enum Msg_Type{
	SYS_COMMAND = 0x01,		//Such as Mode Restore
	CONFIG,				//Config
	SYN_CONTROL,		//synchronization control
	SYN_STATE			//synchronization state
};
enum SynControl{
	INPUT_FLAG_OPEN = 0x08,
	INPUT_FLAG_CLOSE = 0x10,
	INPUT_FLAG_FREEZE = 0x20,
	INPUT_FLAG_UNFREEZE = 0x40
};
struct SynState{
	u8 sm_state;
	u8 comm_state;
	u8 temperature;
	u8 wetness;
	u8 power;
	u8 run_time;
};

typedef cJSON *(*net_response_func)(void *args, char *ret_type);

struct DoorNetOperator {
	char *req_type;
	net_response_func fuc;
	char *res_type;
};

void ESP8266_Init(int baudRate,u16 sendInterval);
void ESP8266_printf (char * Data, ... );
void ESP8266_Sendu8(u8 ch);
void ESP8266_senditoa(u32 d);
u8 ESP8266_ReadNetData(char data[], enum Msg_Type *msg_type, u8 *remarkId, u16 timeout);

u8 ESP8266_NeedSendData(void);
void ESP8266_SendNetData(char *ostream, u16 len, u8 type, u8 remarkId);
void ESP8266_SendCmd(char *ostream,u16 len);
void ESP8266_Send(char *ostream, u16 len, u8 type, u8 remarkId);

void ESP8266_send_syn_status();

void ESP8266_test(void);
u8 GetJsonType(json_t *injson,char *type);
u8 json_find(json_t *injson,char *key,char *ret);
u8 json_add(json_t *injson,char *key,char *value);
u8 ESP8266_Restore();

u8 ESP8266_RecvProcess(void);


cJSON *door_config_get(void *args, char *ret_type);
cJSON *safe_config_get(void *args, char *ret_type);
cJSON *door_config_set(void *args, char *ret_type);
cJSON *safe_config_set(void *args, char *ret_type);

#endif

