#include "esp8266.h"
#include <jansson.h>
#include "usart.h"
#include "string.h"
#include "delay.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include "timer.h"
#include "dma.h"
#include "cJSON.h"
#include "door_fortest.h"


extern struct USART2_RD_QUEUE rdQueue;	//接收队列缓冲区

static struct DoorNetOperator door_net_operator[] = {
	{ "GetApiVersion", 			door_config_get,			"Reply_ApiVersion" },
	{ "GetDoorConfig",			door_config_get,			"Reply_DoorConfig" },
	{ "GetSafeConfig",			safe_config_get,			"Reply_SafeConfig" },
	{ "SetDoorConfig",			door_config_set,			"Reply_SetDoorConfig" },
	{ "SetSafeConfig",			safe_config_set,			"Reply_SetSafeConfig" },
	{NULL,									NULL,									NULL}
};

u8 RestoreFlag = 0;

//Timer Dma
char SendBuf[1024];//需要发送的数据
char ESP8266sData[DMA_SINGLE_LEN_MAX];//DMA发送通道缓冲区
u16 SendCursor = 0xffff;//当前发送位

char token[24]="";

enum Msg_Type msg_type;
enum SynControl syn_control;
struct SynState syn_state;

cJSON *safe_config_get(void *agr, char *ret_type) {
	cJSON *retroot,*data;
	retroot = cJSON_CreateObject();
	cJSON_AddStringToObject(retroot, "type", ret_type);
	cJSON_AddItemToObject(retroot, "data", data = cJSON_CreateObject());
	cJSON_AddStringToObject(data, "token", token);
	return retroot;
}

cJSON *safe_config_set(void *agr, char *ret_type) {
	cJSON *retroot;
	const cJSON *value = cJSON_GetObjectItemCaseSensitive((cJSON *)agr, "Token");
	if (value && cJSON_IsString(value)) {
		//Token = value->string;
	}
	retroot = cJSON_CreateObject();
	cJSON_AddStringToObject(retroot, "type", ret_type);
	return retroot;
}

cJSON *door_config_get(void *agr, char *ret_type) {
	cJSON *retroot, *data=NULL;
	data = to_json();
	retroot = cJSON_CreateObject();
	cJSON_AddStringToObject(retroot, "type", ret_type);
	cJSON_AddItemToObject(retroot, "data", data);
	return retroot;
}
cJSON *door_config_set(void *agr, char *ret_type) {
	cJSON *root = (cJSON*)agr;
	cJSON *retroot,*data;
	retroot = cJSON_CreateObject();
	cJSON_AddStringToObject(retroot, "type", ret_type);
	data=cJSON_GetObjectItem(root,"data");
	from_json(data);
	return retroot;
}


void ESP8266_test()
{
	
}

u8 ESP8266_Start()
{
	ESP8266_RecvProcess();
	return 1;
}
//开始接收网络数据
u8 ESP8266_RecvProcess()
{
	char recvData[1024];
	u8 remarkId;
	//当需要发送数据
	if(ESP8266_NeedSendData())return 1;
	if(RestoreFlag == 1)
	{
		char *data = "restore";
		ESP8266_SendCmd(data,strlen(data));
		RestoreFlag = 0;
	}
	if(!ESP8266_ReadNetData(recvData, &msg_type, &remarkId, 255))return 0;
	if(1 == 1)
	{
		char *ostream=NULL;
		cJSON *root=NULL;
		cJSON *retroot;
		u8 error_code=0;
		switch(msg_type){
			case SYS_COMMAND:
				
				break;
			case CONFIG:
				do{
						u8 i;
						//json_error_t *jerror;
						char *type;
						root=cJSON_Parse(recvData);
						if(root==NULL)goto badjson;
						type=cJSON_GetObjectItem(root,"type")->valuestring;
						//strcpy(type,cJSON_GetObjectItem(root,"type")->valuestring);
						if(type==NULL)
							goto badjson;
						printf("Recvice type:%s\r\n",type);
						for (i = 0; door_net_operator[i].req_type != NULL; i++) {
							if (strcmp(type, door_net_operator[i].req_type) == 0) {
								retroot=door_net_operator[i].fuc(root, door_net_operator[i].res_type);
								if(retroot==NULL)
								{
									retroot=cJSON_CreateObject();
									cJSON_AddStringToObject(retroot, "type", door_net_operator[i].res_type);
									error_code=1;
								}
								break;
							}
						}
				badjson:
						if(type==NULL || door_net_operator[i].req_type == NULL){
							error_code=1;
						}
					}while(0);
				break;
			case SYN_CONTROL:
				printf("recv syn_control:%d\r\n",*recvData);
				syn_control=(enum SynControl)*recvData;
				do{
					int flag=0;
					retroot=cJSON_CreateObject();
					cJSON_AddStringToObject(retroot, "type", "Reply_Control");
					switch(syn_control){
						case INPUT_FLAG_OPEN:
							//handle control
							flag=1;
							break;
						case INPUT_FLAG_CLOSE:
							flag=1;
							break;
						case INPUT_FLAG_FREEZE:
							flag=1;
							break;
						case INPUT_FLAG_UNFREEZE:
							flag=1;
							break;
					}
					if(flag==0)error_code=1;
				}while(0);
				break;
			case SYN_STATE:
				
				break;
		}
		if(error_code==0)
			cJSON_AddStringToObject(retroot, "error_str", "");
		else if(error_code==1){
			if(retroot==NULL){
				goto free_beforeEnd;
			}
			cJSON_AddStringToObject(retroot, "error_str", "None type");
		}
		cJSON_AddNumberToObject(retroot, "error_code", error_code);
		ostream=cJSON_Print(retroot);
		ESP8266_SendNetData(ostream,strlen(ostream),msg_type,remarkId);
free_beforeEnd:
		if(ostream!=NULL)free(ostream);
		if(root!=NULL)cJSON_Delete(root);
		if(retroot!=NULL)cJSON_Delete(retroot);
	}
	return 1;
}

//1S发送一次
void ESP8266_send_syn_status(){
	if(!ESP8266_NeedSendData())
	{
		u8 sbuf[6];
		sbuf[0]=syn_state.sm_state;
		sbuf[1]=syn_state.comm_state;
		sbuf[2]=syn_state.temperature;
		sbuf[3]=syn_state.wetness;
		sbuf[4]=syn_state.power;
		sbuf[5]=syn_state.run_time;
		ESP8266_Send(sbuf,6,SYN_STATE,0);
	}
}

void ESP8266_Init(int baudRate,u16 sendInterval)
{
	u8 i,j;
	char str[3];
	//ESP8266 使用串口1通信
	USART1_Init(baudRate);
	MYDMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)ESP8266sData,DMA_SINGLE_LEN_MAX);
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //使能串口1 DMA通道
	TIM3_Int_Init(sendInterval-1,7199);//10Khz的计数频率，计数到5000为500ms
	Door_Init();
	//获取设备UID,Token
	for(i=0;i<12;i++)
	{
	  j=*(u8*)(UID_BASE_ADDER+sizeof(u8)*i);
		if(j<0x10)
		{
			sprintf(str,"0%x",j);
		}
		else
		{
			sprintf(str,"%x",j);
		}
		j=0;
		while(*(str+j)!=0)
		{
		  if(*(str+j)>='a' && *(str+j)<='f')
			{
				*(str+j)-=32;
			}
			j++;
		}
		strcat(token,str);
	}
//	u8 i=0;
//	for(i=0;i<SERVER_ACCEPT_MAX;i++)
//	{
//		sRecvBuf[i].rear=0;
//		sRecvBuf[i].head=0;
//		sRecvBuf[i].singleLen=0xffff;
//		sRecvBuf[i].cursor= 0;
//	}
//	rdQueue.rear=0;
//	rdQueue.head=0;
//	readATFlag=1;	
}
//重启模块，硬重启
u8 ESP8266_RST()
{
	
	return 0;
}
//软还原
u8 ESP8266_Restore()
{
	ESP8266_printf("");//restore command
	return 1;
}

u8 ESP8266_ReadNetData(char data[], enum Msg_Type *msg_type, u8 *remarkId, u16 timeout)
{
	u8 i;
	u8 ch=1;
	u16 len;
	int tc=0;
	while(rdQueue.head==rdQueue.rear && tc<timeout)//等待数据
	{
		tc+=1;
	}
	if(rdQueue.head==rdQueue.rear)return 0;//计时溢出，没有数据
	len = rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
	len<<=8;
	len |= rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
	*msg_type = (enum Msg_Type)rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
	*remarkId = rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
	len-=4;
	
	printf("ReadNetData length is:%d_remarkId is:%d_type is:%d\r\n",len,*remarkId,*msg_type);
	while(len>0)
	{
		ch=rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
		*data=ch;
		data++;
		len--;
	}
	*data=0;
	if(rdQueue.head>rdQueue.rear)
	{
		rdQueue.head=rdQueue.rear;
		return 0;
	}
	return 1;
}
//Timer控制发送，SendCursor=0xffff没有数据需要发送
u8 ESP8266_NeedSendData()
{
	if(SendCursor==0xffff)
	{
		return 0;
	}
	return 1;
}
/*
*"ZY" + Length(2) + Type(1)  + RemarkId(1) + Data(..) + TotalCheck(1)
*/
void ESP8266_SendNetData(char *ostream, u16 len, u8 type, u8 remarkId)
{
	ESP8266_Send(ostream,len,type,remarkId);
}
void ESP8266_SendCmd(char *ostream,u16 len)
{
	ESP8266_Send(ostream,len,SYS_COMMAND,0);
}
void ESP8266_Send(char *ostream, u16 len, u8 type, u8 remarkId)
{
	u8 TotalCheck = 0;
	u8 headLen = 6;
	u16 i;
	//printf("%s\r\n",ostream);
	memset(SendBuf,0,sizeof(SendBuf));
	sprintf(SendBuf,"ZY%c%c%c%c%s",0,0,0,0,ostream);
	i=2;
	*(SendBuf+i++)=(headLen+len+1)>>8;
	*(SendBuf+i++)=(headLen+len+1)&0x00ff;
	*(SendBuf+i++)=type;
	*(SendBuf+i++)=remarkId;
	for(i=0;i<headLen+len;i++)
	{
	  TotalCheck+=*(SendBuf+i);
	}
	*(SendBuf+i)=TotalCheck;
	printf("length is:%d_type is:%d_remarkId is:%d_totalCheck is:%d\r\n",headLen+len+1,type,remarkId,TotalCheck);
	SendCursor = 0;
}
/*
* %d 发送的是 u32 ，转换成对应的字符发送！！！
*/
void ESP8266_printf (char * Data, ... )
{
	const char *s;
	u32 d;   
	va_list arg_ptr;
	va_start(arg_ptr, Data);//使用指针arg_ptr，遍历堆栈段中的参数列表
	while ( *Data != 0 )
	{				                          
		if ( *Data == 0x5c )  //'\'
		{									  
			switch (*++Data)
			{
				case 'r':							          // \r
				ESP8266_Sendu8(0x0d);
				Data ++;
				break;
				case 'n':							          // \n
				ESP8266_Sendu8(0x0a);	
				Data ++;
				break;
				default:
				Data ++;
				break;
			}			 
		}
		else if ( * Data == '%')
		{									  //
			switch ( *++Data )
			{				
				case 's':										  // %s
				s = va_arg(arg_ptr, const char *);	//取数据，类型为const char*
				for ( ; *s; s++) 
				{
					ESP8266_Sendu8(*s);
				}
				Data++;
				break;
				case 'd':										//%d	十进制的整形数据
				d = va_arg(arg_ptr, u32);
				if(d==0)ESP8266_Sendu8('0');
					else ESP8266_senditoa(d);
				Data++;
				break;
				default:	// %s %d外的占位符，不做处理!!!
				Data++;
				break;
			}		 
		}
		else	ESP8266_Sendu8(*Data++);
	}
	va_end(arg_ptr);
}
void ESP8266_Sendu8(u8 ch)
{
	USART1_Putc(ch);
}
void ESP8266_senditoa(u32 d)
{
	if(d==0)return;
	ESP8266_senditoa(d/10);
	ESP8266_Sendu8(d%10+'0');
}




