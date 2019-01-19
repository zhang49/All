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

extern struct USART2_RD_QUEUE rdQueue;	//接收队列缓冲区

u8 is_lock_enabled=1;
u16 open_stay_time=123;
u16 lock_delay_time=456;
u8 is_detect_enabled=1;
u8 is_always_open=1;
u8 is_double_group=1;
u8 is_auto_close=1;
float run_speed_ratio=0.2;
float run_acc_speed_ratio=0.1;
float study_speed_ratio=0.6;
char token[24]="";

const u8 Send_SingleLen = 20;//单次发送的数据长度
u8 sendBuf[Send_SingleLen];//DMA发送缓冲区
char ESP8266sData[512];
u16 ESP8266sLen = 0;

void ESP8266_test()
{
		char recvData[1024]="{\"type\" : \"GetDoorConfig\",	\"data\" : \"\"}";
	  json_t *injson,*outjson;
		//json_error_t *jerror;
		char *ostream;
		char rjsonbuf[255];
		char temp[10];
		u8 error_code=0;
		outjson=json_object();
	injson=json_loads(recvData, JSON_DECODE_ANY,NULL);
	
	if(json_find(injson,"type",rjsonbuf))
		{
	if(strstr(rjsonbuf,"GetDoorConfig"))
	{
		printf("%s\r\n",ostream);
	}
}
	strcpy(recvData,"{\"type\" : \"SetDoorConfig\",  	\"error_code\" : \"0\",	\"error_str\" : \"\",    \"data\" : {  		      \"is_lock_enabled\" : \"1\",      \"open_stay_time\" : \"66\",      \"lock_delay_time\" : \"66\",       \"is_detect_enabled\" : \"1\",      \"is_always_open\" : \"1\",      \"is_double_group\" : \"1\",      \"run_speed_ratio\" : \"6.66666\",      \"run_acc_speed_ratio\" : \"6.66666\",      \"study_speed_ratio\" : \"6.66666\"}}");
	injson=json_loads(recvData, JSON_DECODE_ANY,NULL);
if(json_find(injson,"type",rjsonbuf))
		{
	if(strstr(rjsonbuf,"SetDoorConfig"))
	{
		
		printf("%s\r\n",ostream);
		
	}
}
	/*
	char *out;
	char rjsonbuf[255];
	json_t *outjson,*injson;
	char ret[255];
	u16 sdlen;
	outjson=json_object();
	
	json_add(outjson,"type","request");
	json_add(outjson,"data.id","#1");
	json_add(outjson,"data.version","1.1.1");
	json_add(outjson,"data.isopen","false");
	json_add(outjson,"data2.id","#1");
	json_add(outjson,"data2.version","1.1.1");
	json_add(outjson,"data2.isopen","false");
	json_add(outjson,"data3.id","#1");
	json_add(outjson,"data3.version","1.1.1");
	json_add(outjson,"data3.isopen","false");
	out = json_dumps(outjson, JSON_COMPACT|JSON_INDENT(0));
	sdlen=strlen(out);
	
	ESP8266_printf("AT+CIPSEND=%d,%d\r\n",0,sdlen+2);
	delay_ms(800);
	ESP8266_Sendu8(sdlen>>8);//先发高位
	ESP8266_Sendu8(sdlen);
	ESP8266_printf("%s",out);
	
	printf("send %d\r\n",(u8)sdlen);
	printf("send %d\r\n",sdlen);
	free(out);
		
	//创建新json_t时，引用计数为1
	//调用json_decref()一次，参数引用计数 -1 ，当计数为0，自动释放引用
	json_decref(outjson);
	json_decref(injson);
	
	return;
	
	injson=json_loads(out, JSON_DECODE_ANY,NULL);
	json_find(injson,"data.isopen",ret);
	
	printf("find data.isopen :%s\r\n",ret);

	
	//json_find(injson,"data.version",ret);
	
	//printf("------------------ret is :           %s\r\n",ret);
	
	//当重新赋值为另一个引用，需要释放原先的，否则会造成内存泄漏
	//json_decref(injson);
	//injson=json_loads(in, JSON_DECODE_ANY,NULL);
		
	//The return value must be freed by the caller using free().
	//out = json_dumps(injson, JSON_DECODE_ANY);
	//printf("Test json to string is:%s\r\n",out);
	//free(out);
	
	//json_t *injson=json_object();
	//json_add(injson,"type","request");
	
	//is pass tesing
	//json_find(injson,"data.id");
	
//printf("not crash.\r\n");
//	if(getJsonData(injson,"type",NULL,rjsonbuf))
//	{
//		printf("getJson data.id is:%s\r\n",rjsonbuf);
//	}
//	else
//	{
//		printf("data.id not found\r\n");
//	}

*/
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
	char recvData[512];
	
//	//测试
//	//ESP8266_test();
//	json_t *injson,*outjson;
//	//json_error_t *jerror;
//	char *ostream;
//	char rjsonbuf[255];
//	char temp[10];
//	u8 error_code=0;
//	outjson=json_object();
//	json_add(outjson,"user_data","none");
//	json_add(outjson,"type","Reply_GetApiVersion");
//	json_add(outjson,"api_version","2");
//	json_add(outjson,"app_version","3");
//	
//	error_code=0;
//	sprintf(temp,"%d",error_code);
//	json_add(outjson,"error_code",temp);
//	json_add(outjson,"error_str","");
//	ostream=json_dumps(outjson, JSON_ENCODE_ANY|JSON_COMPACT);
//	ESP8266_SendNetData(ostream,strlen(ostream));
					
	if(!ESP8266_ReadNetData(recvData,255))return 0;
	do
	{
		json_t *injson,*outjson;
		//json_error_t *jerror;
		char *ostream;
		char rjsonbuf[255];
		char temp[10];
		u8 error_code=0;
		printf("Recvice Data:%s\r\n",recvData);
		//测试
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
					ESP8266_SendNetData(ostream,strlen(ostream));
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
				ESP8266_SendNetData(ostream,strlen(ostream));
			}
			else if(strstr(rjsonbuf,"GetDoorConfig"))
			{
				if(json_find(injson,"user_data",rjsonbuf))
				{
					json_add(outjson,"user_data",rjsonbuf);
				}
				
				json_add(outjson,"type","Reply_GetDoorConfig");
				
				sprintf(temp,"%d",is_auto_close);
				json_add(outjson,"data.is_auto_close",temp);
				sprintf(temp,"%d",is_lock_enabled);
				json_add(outjson,"data.is_lock_enabled",temp);
				sprintf(temp,"%d",open_stay_time);
				json_add(outjson,"data.open_stay_time",temp);
				sprintf(temp,"%d",lock_delay_time);
				json_add(outjson,"data.lock_delay_time",temp);
				sprintf(temp,"%d",is_detect_enabled);
				json_add(outjson,"data.is_detect_enabled",temp);
				sprintf(temp,"%d",is_always_open);
				json_add(outjson,"data.is_always_open",temp);
				sprintf(temp,"%d",is_double_group);
				json_add(outjson,"data.is_double_group",temp);
				
				sprintf(temp,"%.1f",run_speed_ratio);
				json_add(outjson,"data.run_speed_ratio",temp);
				sprintf(temp,"%.1f",run_acc_speed_ratio);
				json_add(outjson,"data.run_acc_speed_ratio",temp);
				sprintf(temp,"%.1f",study_speed_ratio);
				json_add(outjson,"data.study_speed_ratio",temp);
				
				error_code=0;
				sprintf(temp,"%d",error_code);
				json_add(outjson,"error_code",temp);
				json_add(outjson,"error_str","");
				ostream=json_dumps(outjson, JSON_ENCODE_ANY|JSON_COMPACT);
				ESP8266_SendNetData(ostream,strlen(ostream));
			}
			else if(strstr(rjsonbuf,"SetDoorConfig"))
			{
				if(json_find(injson,"user_data",rjsonbuf))
				{
					json_add(outjson,"user_data",rjsonbuf);
				}
				
				json_add(outjson,"type","Reply_SetDoorConfig");
				
				json_find(injson,"data.is_auto_close",rjsonbuf);
				is_auto_close=atoi(rjsonbuf);
				json_find(injson,"data.is_lock_enabled",rjsonbuf);
				is_lock_enabled=atoi(rjsonbuf);
				json_find(injson,"data.open_stay_time",rjsonbuf);
				open_stay_time=atoi(rjsonbuf);
				json_find(injson,"data.lock_delay_time",rjsonbuf);
				lock_delay_time=atoi(rjsonbuf);
				json_find(injson,"data.is_detect_enabled",rjsonbuf);
				is_detect_enabled=atoi(rjsonbuf);
				json_find(injson,"data.is_always_open",rjsonbuf);
				is_always_open=atoi(rjsonbuf);
				json_find(injson,"data.is_double_group",rjsonbuf);
				is_double_group=atoi(rjsonbuf);
				
				json_find(injson,"data.run_speed_ratio",rjsonbuf);
				run_speed_ratio=atof(rjsonbuf);
				json_find(injson,"data.run_acc_speed_ratio",rjsonbuf);
				run_acc_speed_ratio=atof(rjsonbuf);
				json_find(injson,"data.study_speed_ratio",rjsonbuf);
				study_speed_ratio=atof(rjsonbuf);

				/*printf("open_stay_time :%d\r\n",open_stay_time);
				printf("lock_delay_time :%d\r\n",lock_delay_time);
				printf("is_detect_enabled :%d\r\n",is_detect_enabled);
				printf("is_always_open :%d\r\n",is_always_open);
				printf("is_double_group :%d\r\n",is_double_group);
				printf("open_stay_time :%d\r\n",open_stay_time);
				
				printf("run_speed_ratio :%f\r\n",run_speed_ratio);
				printf("run_acc_speed_ratio :%f\r\n",run_acc_speed_ratio);
				printf("study_speed_ratio :%f\r\n",study_speed_ratio);
				*/
				
				error_code=0;
				sprintf(temp,"%d",error_code);
				json_add(outjson,"error_code",temp);
				json_add(outjson,"error_str","");
				ostream=json_dumps(outjson, JSON_ENCODE_ANY|JSON_COMPACT);
				ESP8266_SendNetData(ostream,strlen(ostream));
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
				ESP8266_SendNetData(ostream,strlen(ostream));
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
				ESP8266_SendNetData(ostream,strlen(ostream));
			}
		}
		else
		{
			printf("encode json error!");
		}
		free(ostream);
		json_decref(injson);
		json_decref(outjson);
	}
	while(0);
	return 1;
}

extern struct USART2_RD_QUEUE rdQueue;	//队列缓冲区

void ESP8266_Init(int baudRate,u32 sendInterval)
{
	u8 i,j;
	char temp;
	char str[3];
	//ESP8266 使用串口1通信
	USART1_Init(baudRate);
	MYDMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)ESP8266sData,512);
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //使能串口1 DMA通道
	//TIM3_Int_Init(sendInterval*10-1,7199);//10Khz的计数频率，计数到5000为500ms
	//获取设备UID
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

//重启模块，硬重启？
u8 ESP8266_RST()
{
	
	return 0;
}
//软还原？
u8 ESP8266_Restore()
{
	ESP8266_printf("");//restore command
	return 1;
}

u8 ESP8266_ReadNetData(char data[],u16 timeout)
{
	u8 ch=1;
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
	while(ch!=0 && rdQueue.head<rdQueue.rear)
	{
		ch=rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
		*data=ch;
		//printf("data ch:%c\r\n",ch);
		data++;
	}
	*data=0;
	//printf("OVER...head:%d,rear:%d\r\n",rdQueue.head,rdQueue.rear);
	return 1;
}

void ESP8266_SendNetData(char *ostream,u16 len)
{
	u16 i=0;;
	//timer 3 检查ESP8266sLen，当其不为0则发送，发送完毕置0
	for(;i<len;i++)*(ESP8266sData+i)=*(ostream+i);
	*(ESP8266sData+i++)= 0xff;
	*(ESP8266sData+i++)= '+';
	ESP8266sLen = i;
	MYDMA_Enable(DMA1_Channel4,ESP8266sLen);//开始一次DMA传输
	//ESP8266_Sendu8(len>>8);//先发高位
	//ESP8266_Sendu8(len);
	//printf("*****SEND DATA TO NET:\r\n%s\r\n*****\r\n",ostream);
	//ESP8266_printf("%s+++",ostream);
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




