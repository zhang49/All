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

struct config_def{
    //????
    int hall_distance;          //??hall?????,???????????,??mm,????:0-10000
    int motor_len;              //??????,??mm,????:100-1000
    float max_speed;            //??????,??mm/s,????:100-1000
    float max_acc_speed;        //?????,mm/s2,????:10-10000
    int max_current;            //????,????(????),????:500-10000
    int limit_current;          //????,????(????)????:500-10000

    //????
    int motion_range;           //???????,????:100-10000
    int is_lock_enabled;       //?????
    float lock_delay_time;      //???????,??s,????:0-100

    int is_auto_close;         //??????
    float open_stay_time;       //??????,??s,????:0-100

    int is_detect_ir;          //????????
    int is_double_group;       //??????

    int is_detect_move;        //????????
    int move_distance;          //?????,??mm,????:0-100

    int is_detect_resist;      //????
    float resist_time;          //????,??s,????:0.01-1

    float open_speed_ratio;     //??????,???????,????:0.0-1.0
    float close_speed_ratio;    //??????,???????,????:0.0-1.0
    float study_speed_ratio;    //??????,???????,????:0.0-1.0
    float min_speed_ratio;      //??????,???????,????:0.0-1.0
    float acc_speed_ratio;      //?????,????????,????:0.0-1.0
    float dec_speed_ratio;      //?????,????????,????:0.0-1.0
    float run_low_speed_ratio;  //??????,????????,???????,???????????,????:0.0-1.0
    float open_pre_dec_speed_ratio; //???????,??????(????????????),????:0.0-1.0
    float close_pre_dec_speed_ratio; //???????,??????,????:0.0-1.0
}doorConfig;

				
//门数据等
u8 is_lock_enabled=1;
u16 open_stay_time=123;
u16 lock_delay_time=456;

u8 is_detect_ir=1;
//u8 is_always_open=1;
u8 is_double_group=1;
u8 is_auto_close=1;

//float run_speed_ratio=0.2;
//float run_acc_speed_ratio=0.1;

float study_speed_ratio=0.6;
char token[24]="";


extern struct USART2_RD_QUEUE rdQueue;	//接收队列缓冲区

u8 RestoreFlag = 0;

//Timer Dma
char SendBuf[1024];//需要发送的数据
char ESP8266sData[255];//DMA发送通道缓冲区
u16 SendCursor = 0xffff;//当前发送位

void ESP8266_test()
{
	json_t *injson,*outjson;
	//json_error_t *jerror;
	char *ostream;
	char rjsonbuf[255];
	char temp[10];
	u8 error_code=0;
	outjson=json_object();
	
	json_add(outjson,"type","Reply_Command");
	json_add(outjson,"data","");
	
	error_code=0;
	sprintf(temp,"%d",error_code);
	json_add(outjson,"error_code",temp);
	json_add(outjson,"error_str","");
	ostream=json_dumps(outjson, JSON_ENCODE_ANY|JSON_COMPACT);
	ESP8266_SendNetData(ostream, 1, strlen(ostream));
}

u8 ESP8266_Start()
{
	ESP8266_RecvProcess();
	return 1;
}
u8 json_add(json_t *injson,char *key,char *value)
{
	if(strstr(key,"."))
	{
		json_t *tempobj;
		char fkey[20];
		u8 i=0;
		for(i=0;*key!='.';i++,key++)
		*(fkey+i)=*key;
		*(fkey+i)=0;
		key++;
		tempobj=json_object_get(injson,fkey);
		if(tempobj==NULL)
		{
			//printf("is NULL\r\n");
			//new jsonobject
			tempobj=json_object();
			//json_string(value) 会新分配内存，不盗取则需要释放
			//set_new 盗取value 当value时新建的，在调用后无法使用
			json_object_set_new(tempobj, key, json_string(value));
			json_object_set_new(injson,fkey,tempobj);
		}
		else
		{
			//printf("NOT is NULL\r\n");
			json_object_set_new(tempobj, key, json_string(value));
			json_object_set(injson,fkey,tempobj);
		}
		return 1;
	}
	json_object_set_new(injson,key,json_string(value));
}
//参数json_t类型作为返回值，赋值失败，暂时
u8 json_find(json_t *injson,char *key,char *ret)
{
		//printf("test\r\n");
		if(json_is_array(injson) && json_array_size(injson)==0)return 0;
		if(json_is_object(injson) && json_object_size(injson)==0)return 0;
    if(json_is_object(injson))
    {
				char keytemp[20];
				const char *k;
				json_t *v;
				int i=0;
				//printf("key is:%s\r\n",key);
        if(strstr(key,"."))
				{
					for(i=0;*key!='.';i++,key++)
					*(keytemp+i)=*key;
					*(keytemp+i)=0;
				}
        else
            strcpy(keytemp,key);
				//printf("keytemp is:%s\r\n",keytemp);
				json_object_foreach(injson, k, v)
				{
					//printf("k is:%s\r\n",k);
					if(strlen(k)==strlen(keytemp) && strstr(k,keytemp))
					{
							if(strlen(keytemp)==strlen(key) && strstr(keytemp,key))
							{
								//终于找到了
								strcpy(ret,json_string_value(v));
								return 1;
							}
							else
							{
								key++;//去掉'.'
								//printf("backkey is:%s\r\n",key);
								if(json_find(json_object_get(injson,k),key,ret))
								{
									return 1;
								}
							}
					}
				}
        return 0;
    }
    else if(json_is_array(injson))
    {
			u8 i=0;
			for(;i<json_array_size(injson);i++)
			if(json_find(json_array_get(injson,i),key,ret))
			{
					return 1;
			}
    }
    return 0;
}

u8 GetJsonType(json_t *injson,char *type)
{
		if(!json_find(injson,"type",type))
		{
			printf("GetJsonType error\r\n");
			return 0;
		}
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
	if(!ESP8266_ReadNetData(recvData, &remarkId, 255))return 0;
	if(1 == 1)
	{
		json_t *injson,*outjson;
		//json_error_t *jerror;
		char *ostream;
		char rjsonbuf[255];
		char temp[10];
		u8 error_code=0;
		printf("Recvice Data:%s\r\n",recvData);
		//测试
		doorConfig.hall_distance = 1;
		doorConfig.motor_len = 1;
		doorConfig.max_speed = 0.1;
		doorConfig.max_acc_speed = 0.1;
		doorConfig.max_current = 1;
		doorConfig.limit_current = 1;
		doorConfig.motion_range = 1;
		doorConfig.is_lock_enabled = 1;
		doorConfig.lock_delay_time = 0.1;
		doorConfig.is_auto_close = 1;
		doorConfig.open_stay_time = 0.1;
		doorConfig.is_detect_ir = 1;
		doorConfig.is_double_group = 1;
		doorConfig.is_detect_move = 1;
		doorConfig.move_distance = 1;
		doorConfig.is_detect_resist = 1;
		doorConfig.resist_time = 0.1;
		doorConfig.open_speed_ratio = 0.1;
		doorConfig.close_speed_ratio = 0.1;
		doorConfig.study_speed_ratio = 0.1;
		doorConfig.min_speed_ratio = 0.1;
		doorConfig.acc_speed_ratio = 0.1;
		doorConfig.dec_speed_ratio = 0.1;
		doorConfig.run_low_speed_ratio = 0.1;
		doorConfig.open_pre_dec_speed_ratio = 0.1;
		doorConfig.close_pre_dec_speed_ratio = 0.1;
		//ESP8266_test();
		outjson=json_object();
		injson=json_loads(recvData, JSON_DECODE_ANY,NULL);
		if(json_find(injson,"type",rjsonbuf))
		{
			//printf("Message Type is:%s\r\n",rjsonbuf);
			if(strstr(rjsonbuf,"GetApiVersion"))
			{
				if(json_find(injson,"data",rjsonbuf))
				{
					if(json_find(injson,"user_data",rjsonbuf))
					{
						json_add(outjson,"user_data",rjsonbuf);
					}
				
					json_add(outjson,"type","Reply_GetApiVersion");
					json_add(outjson,"api_version","2");
					json_add(outjson,"app_version","3");
					
					error_code=0;
					sprintf(temp,"%d",error_code);
					json_add(outjson,"error_code",temp);
					json_add(outjson,"error_str","");
					ostream=json_dumps(outjson, JSON_ENCODE_ANY|JSON_COMPACT);
					ESP8266_SendNetData(ostream, remarkId, strlen(ostream));
				}
			}
			else if(strstr(rjsonbuf,"Heartbeat"))
			{
				if(json_find(injson,"user_data",rjsonbuf))
				{
					json_add(outjson,"user_data",rjsonbuf);
				}
				json_add(outjson,"type","Heartbeat");
				
				error_code=0;
				sprintf(temp,"%d",error_code);
				json_add(outjson,"error_code",temp);
				json_add(outjson,"error_str","");
				ostream=json_dumps(outjson, JSON_ENCODE_ANY|JSON_COMPACT);
				ESP8266_SendNetData(ostream, remarkId, strlen(ostream));
			}
			else if(strstr(rjsonbuf,"GetDoorConfig"))
			{
				if(json_find(injson,"user_data",rjsonbuf))
				{
					json_add(outjson,"user_data",rjsonbuf);
				}
				json_add(outjson,"type","Reply_GetDoorConfig");
				
				sprintf(temp,"%d",doorConfig.hall_distance);
				json_add(outjson,"data.hall_distance",temp);
				
				sprintf(temp,"%d",doorConfig.motor_len);
				json_add(outjson,"data.motor_len",temp);
				
				sprintf(temp,"%.1f",doorConfig.max_speed);
				json_add(outjson,"data.max_speed",temp);
				
				sprintf(temp,"%.1f",doorConfig.max_acc_speed);
				json_add(outjson,"data.max_acc_speed",temp);
				
				sprintf(temp,"%d",doorConfig.max_current);
				json_add(outjson,"data.max_current",temp);
				
				sprintf(temp,"%d",doorConfig.limit_current);
				json_add(outjson,"data.limit_current",temp);
				
				sprintf(temp,"%d",doorConfig.motion_range);
				json_add(outjson,"data.motion_range",temp);
				
				sprintf(temp,"%d",doorConfig.is_lock_enabled);
				json_add(outjson,"data.is_lock_enabled",temp);
				
				sprintf(temp,"%.1f",doorConfig.lock_delay_time);
				json_add(outjson,"data.lock_delay_time",temp);
				
				sprintf(temp,"%d",doorConfig.is_auto_close);
				json_add(outjson,"data.is_auto_close",temp);
				
				sprintf(temp,"%.1f",doorConfig.open_stay_time);
				json_add(outjson,"data.open_stay_time",temp);
				
				sprintf(temp,"%d",doorConfig.is_detect_ir);
				json_add(outjson,"data.is_detect_ir",temp);
				
				sprintf(temp,"%d",doorConfig.is_double_group);
				json_add(outjson,"data.is_double_group",temp);
				
				sprintf(temp,"%d",doorConfig.is_detect_move);
				json_add(outjson,"data.is_detect_move",temp);
				
				sprintf(temp,"%d",doorConfig.move_distance);
				json_add(outjson,"data.move_distance",temp);
				
				sprintf(temp,"%d",doorConfig.is_detect_resist);
				json_add(outjson,"data.is_detect_resist",temp);
				
				sprintf(temp,"%.1f",doorConfig.resist_time);
				json_add(outjson,"data.resist_time",temp);
				
				sprintf(temp,"%.1f",doorConfig.open_speed_ratio);
				json_add(outjson,"data.open_speed_ratio",temp);
				
				sprintf(temp,"%.1f",doorConfig.close_speed_ratio);
				json_add(outjson,"data.close_speed_ratio",temp);
				
				sprintf(temp,"%.1f",doorConfig.study_speed_ratio);
				json_add(outjson,"data.study_speed_ratio",temp);
				
				sprintf(temp,"%.1f",doorConfig.min_speed_ratio);
				json_add(outjson,"data.min_speed_ratio",temp);
				
				sprintf(temp,"%.1f",doorConfig.acc_speed_ratio);
				json_add(outjson,"data.acc_speed_ratio",temp);
				
				sprintf(temp,"%.1f",doorConfig.dec_speed_ratio);
				json_add(outjson,"data.dec_speed_ratio",temp);
				
				sprintf(temp,"%.1f",doorConfig.run_low_speed_ratio);
				json_add(outjson,"data.run_low_speed_ratio",temp);
				
				sprintf(temp,"%.1f",doorConfig.open_pre_dec_speed_ratio);
				json_add(outjson,"data.open_pre_dec_speed_ratio",temp);
				
				sprintf(temp,"%.1f",doorConfig.close_pre_dec_speed_ratio);
				json_add(outjson,"data.close_pre_dec_speed_ratio",temp);
								
				error_code=0;
				sprintf(temp,"%d",error_code);
				json_add(outjson,"error_code",temp);
				json_add(outjson,"error_str","");
				ostream=json_dumps(outjson, JSON_ENCODE_ANY|JSON_COMPACT);
				ESP8266_SendNetData(ostream, remarkId, strlen(ostream));
			}
			else if(strstr(rjsonbuf,"SetDoorConfig"))
			{
				if(json_find(injson,"user_data",rjsonbuf))
				{
					json_add(outjson,"user_data",rjsonbuf);
				}
				
				json_add(outjson,"type","Reply_SetDoorConfig");
				
				doorConfig.hall_distance = 1;
				json_find(injson,"data.hall_distance",rjsonbuf);
				doorConfig.hall_distance = atoi(rjsonbuf);
				
				
				doorConfig.motor_len = 1;
				json_find(injson,"data.motor_len",rjsonbuf);
				doorConfig.motor_len = atoi(rjsonbuf);

				doorConfig.max_speed = 0.1;
				json_find(injson,"data.max_speed",rjsonbuf);
				doorConfig.max_speed = atof(rjsonbuf);
						
				doorConfig.max_acc_speed = 0.1;
				json_find(injson,"data.max_acc_speed",rjsonbuf);
				doorConfig.max_acc_speed = atof(rjsonbuf);
						
				doorConfig.max_current = 1;
				json_find(injson,"data.max_current",rjsonbuf);
				doorConfig.max_current = atoi(rjsonbuf);
						
				doorConfig.limit_current = 1;
				json_find(injson,"data.limit_current",rjsonbuf);
				doorConfig.limit_current = atoi(rjsonbuf);
				
				doorConfig.motion_range = 1;
				json_find(injson,"data.motion_range",rjsonbuf);
				doorConfig.motion_range = atoi(rjsonbuf);
				
				doorConfig.is_lock_enabled = 1;
				json_find(injson,"data.is_lock_enabled",rjsonbuf);
				doorConfig.is_lock_enabled = atoi(rjsonbuf);
				
				doorConfig.lock_delay_time = 0.1;
				json_find(injson,"data.lock_delay_time",rjsonbuf);
				doorConfig.lock_delay_time = atof(rjsonbuf);
				
				doorConfig.is_auto_close = 1;
				json_find(injson,"data.is_auto_close",rjsonbuf);
				doorConfig.is_auto_close = atoi(rjsonbuf);
				
				doorConfig.open_stay_time = 0.1;
				json_find(injson,"data.open_stay_time",rjsonbuf);
				doorConfig.open_stay_time = atof(rjsonbuf);
				
				doorConfig.is_detect_ir = 1;
				json_find(injson,"data.is_detect_ir",rjsonbuf);
				doorConfig.is_detect_ir = atoi(rjsonbuf);
				
				doorConfig.is_double_group = 1;
				json_find(injson,"data.is_double_group",rjsonbuf);
				doorConfig.is_double_group = atoi(rjsonbuf);
				
				doorConfig.is_detect_move = 1;
				json_find(injson,"data.is_detect_move",rjsonbuf);
				doorConfig.is_detect_move = atoi(rjsonbuf);
				
				doorConfig.move_distance = 1;
				json_find(injson,"data.move_distance",rjsonbuf);
				doorConfig.move_distance = atoi(rjsonbuf);
				
				doorConfig.is_detect_resist = 1;
				json_find(injson,"data.is_detect_resist",rjsonbuf);
				doorConfig.is_detect_resist = atoi(rjsonbuf);
				
				doorConfig.resist_time = 0.1;
				json_find(injson,"data.resist_time",rjsonbuf);
				doorConfig.resist_time = atof(rjsonbuf);
				
				doorConfig.open_speed_ratio = 0.1;
				json_find(injson,"data.open_speed_ratio",rjsonbuf);
				doorConfig.open_speed_ratio = atof(rjsonbuf);
				
				doorConfig.close_speed_ratio = 0.1;
				json_find(injson,"data.close_speed_ratio",rjsonbuf);
				doorConfig.close_speed_ratio = atof(rjsonbuf);
				
				doorConfig.study_speed_ratio = 0.1;
				json_find(injson,"data.study_speed_ratio",rjsonbuf);
				doorConfig.study_speed_ratio = atof(rjsonbuf);
				
				doorConfig.min_speed_ratio = 0.1;
				json_find(injson,"data.min_speed_ratio",rjsonbuf);
				doorConfig.min_speed_ratio = atof(rjsonbuf);
				
				doorConfig.acc_speed_ratio = 0.1;
				json_find(injson,"data.acc_speed_ratio",rjsonbuf);
				doorConfig.acc_speed_ratio = atof(rjsonbuf);
				
				doorConfig.dec_speed_ratio = 0.1;
				json_find(injson,"data.dec_speed_ratio",rjsonbuf);
				doorConfig.dec_speed_ratio = atof(rjsonbuf);
				
				doorConfig.run_low_speed_ratio = 0.1;
				json_find(injson,"data.run_low_speed_ratio",rjsonbuf);
				doorConfig.run_low_speed_ratio = atof(rjsonbuf);
				
				doorConfig.open_pre_dec_speed_ratio = 0.1;
				json_find(injson,"data.open_pre_dec_speed_ratio",rjsonbuf);
				doorConfig.open_pre_dec_speed_ratio = atof(rjsonbuf);
				
				doorConfig.close_pre_dec_speed_ratio = 0.1;
				json_find(injson,"data.close_pre_dec_speed_ratio",rjsonbuf);
				doorConfig.close_pre_dec_speed_ratio = atof(rjsonbuf);
				
				//printf("study_speed_ratio :%f\r\n",study_speed_ratio);
					
				
				error_code=0;
				sprintf(temp,"%d",error_code);
				json_add(outjson,"error_code",temp);
				json_add(outjson,"error_str","");
				ostream=json_dumps(outjson, JSON_ENCODE_ANY|JSON_COMPACT);
				ESP8266_SendNetData(ostream, remarkId, strlen(ostream));
			}
			else if(strstr(rjsonbuf,"GetSafeConfig"))
			{
				if(json_find(injson,"user_data",rjsonbuf))
				{
					json_add(outjson,"user_data",rjsonbuf);
				}
				json_add(outjson,"type","Reply_GetSafeConfig");
				json_add(outjson,"data.token",token);
				
				error_code=0;
				sprintf(temp,"%d",error_code);
				json_add(outjson,"error_code",temp);
				json_add(outjson,"error_str","");
				ostream=json_dumps(outjson, JSON_ENCODE_ANY|JSON_COMPACT);
				ESP8266_SendNetData(ostream, remarkId, strlen(ostream));
			}
			else if(strstr(rjsonbuf,"Command"))
			{
				if(json_find(injson,"user_data",rjsonbuf))
				{
					json_add(outjson,"user_data",rjsonbuf);
				}
				json_add(outjson,"type","Reply_Command");
				json_add(outjson,"data","");
				
				error_code=0;
				sprintf(temp,"%d",error_code);
				json_add(outjson,"error_code",temp);
				json_add(outjson,"error_str","");
				ostream=json_dumps(outjson, JSON_ENCODE_ANY|JSON_COMPACT);
				ESP8266_SendNetData(ostream, remarkId, strlen(ostream));
			}
		}
		else
		{
			printf("encode json error!\r\n");
		}
		free(ostream);
		json_decref(injson);
		json_decref(outjson);
	}
	return 1;
}


void ESP8266_Init(int baudRate,u16 sendInterval)
{
	u8 i,j;
	char temp;
	char str[3];
	//ESP8266 使用串口1通信
	USART1_Init(baudRate);
	MYDMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)ESP8266sData,512);
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //使能串口1 DMA通道
	TIM3_Int_Init(sendInterval-1,7199);//10Khz的计数频率，计数到5000为500ms
	
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
/////////////AT+UART=115200,8,1,0,3设置波特率，待测

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

u8 ESP8266_ReadNetData(char data[] ,u8 *remarkId, u16 timeout)
{
	u8 ch=1;
	u8 type=0;
	u16 len;
	int tc=0;
	char *head=data;
	//printf("tc:%d,timeout:%d\r\n",tc,tenMsTimes);
	while(rdQueue.head==rdQueue.rear && tc<timeout)//等待数据
	{
		tc+=1;
	}
	if(rdQueue.head==rdQueue.rear)return 0;//计时溢出，没有数据
	//printf("____tc:%d,timeout:%d\r\n",tc,tenMsTimes);
	//printf("head:%d,rear:%d\r\n",rdQueue.head,rdQueue.rear);
	if(data==NULL)return 1;
	len = rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
	len<<=8;
	len |= rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
	*remarkId = rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
	type = rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
	len-=4;
	printf("ReadNetData length is:%d_remarkId is:%d_type is:%d\r\n",len,*remarkId,type);
	while(len>0)
	{
		ch=rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
		*data=ch;
		//printf("data ch:%c\r\n",ch);
		data++;
		len--;
	}
	*data=0;
	if(rdQueue.head>rdQueue.rear)
	{
		rdQueue.head=rdQueue.rear;
		return 0;
	}
	//printf("OVER...head:%d,rear:%d\r\n",rdQueue.head,rdQueue.rear);
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
*"ZY" + RemarkId(1) + Type(1) + Length(2) + Data(..) + TotalCheck(1)
*
*"ZY" + RemarkId(1) + Length(2) + Type(1) + Data(..) + TotalCheck(1)
*/
void ESP8266_SendNetData(char *ostream,u8 remarkId,u16 len)
{
	ESP8266_Send(ostream,remarkId,0x02,len);
}
void ESP8266_SendCmd(char *ostream,u16 len)
{
	ESP8266_Send(ostream,0xff,0x01,len);
}
void ESP8266_Send(char *ostream,u8 remarkId,u8 type,u16 len)
{
	u8 TotalCheck = 0;
	u8 headLen = 6;
	u16 i;
	//printf("%s\r\n",ostream);
	memset(SendBuf,0,sizeof(SendBuf));
	sprintf(SendBuf,"ZY%c%c%c%c%s",0,0,0,0,ostream);
	*(SendBuf+2)=remarkId;
	
	if(((headLen+len+1)&0xff00)==0)*(SendBuf+3)=0xff;
	else *(SendBuf+3)=(headLen+len+1)>>8;
	*(SendBuf+4)=(headLen+len+1)&0x00ff;
	
	*(SendBuf+5)=type;
	for(i=0;i<headLen+len;i++)
	{
	  TotalCheck+=*(SendBuf+i);
	}
	printf("length is:%d_totalCheck is:%d\r\n",headLen+len+1,TotalCheck);
	
	printf("%s\r\n",SendBuf);
	sprintf(SendBuf,"%s%c",SendBuf,TotalCheck);
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




