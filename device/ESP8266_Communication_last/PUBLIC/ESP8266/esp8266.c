#include "esp8266.h"
#include <jansson.h>
#include "usart.h"
#include "string.h"
#include "delay.h"
#include <stdarg.h>
#include <stdint.h>

extern struct USART2_RD_QUEUE rdQueue;	//队列缓冲区

void ESP8266_test()
{
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
}

u8 ESP8266_Start()
{
	ESP8266_RecvProcess();
	return 1;
}
/*
*ESP8266 AP模式下，开启TCP服务器，接收数据
*AT+ CIPSERVER=<mode>[,<port>]  必须AT+CIPMODE=0非透传
*AT+ CIPMUX=1 时才能开启server模式
*/

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
	if(!ESP8266_ReadNetData(recvData,255))return 0;
	do
	{
		json_t *injson,*outjson;
		//json_error_t *jerror;
		char *ostream;
		char rjsonbuf[255];
		printf("Recvice Data:%s\r\n",recvData);
		//测试
		//ESP8266_test();
		outjson=json_object();
		injson=json_loads(recvData, JSON_DECODE_ANY,NULL);
		if(json_find(injson,"type",rjsonbuf))
		{
			printf("Message Type is:%s\r\n",rjsonbuf);
			if(strstr(rjsonbuf,"Request_GetApiVersion"))
			{
				if(json_find(injson,"data",rjsonbuf))
				{
					json_add(outjson,"type","Reply_GetApiVersion");
					json_add(outjson,"api_version","2");
					json_add(outjson,"app_version","3");
					ostream=json_dumps(outjson, JSON_DECODE_ANY);
					ESP8266_SendNetData(ostream);
				}
			}
			else if(strstr(rjsonbuf,"Request_Hearbet"))
			{
				if(json_find(injson,"data.time_tick",rjsonbuf))
				{
					printf("data.time_tick :%s\r\n",rjsonbuf);
				}
			}
			else if(strstr(rjsonbuf,"Request_Getconfig"))
			{
				json_add(outjson,"type","Reply_Getconfig");
				json_add(outjson,"RunMode","0");
				json_add(outjson,"has_lock","1");
				json_add(outjson,"open_stay_time","3");
				json_add(outjson,"lock_delay_time","4");
				ostream=json_dumps(outjson, JSON_DECODE_ANY);
				ESP8266_SendNetData(ostream);
			}
			else if(strstr(rjsonbuf,"Request_ESP8266SetConfig"))
			{
				if(json_find(injson,"data.mode",rjsonbuf))
				{
					
				}
				//不需要保存至STM32 FALSH，ESP8266保存
			}
			else if(strstr(rjsonbuf,"Request_ESP8266SetRestore"))
			{
				
			}
			else if(strstr(rjsonbuf,"Request_SetConfig"))
			{
				json_find(injson,"data.RunMode",rjsonbuf);
				printf("data.RunMode :%s\r\n",rjsonbuf);
				json_find(injson,"data.has_lock",rjsonbuf);
				printf("data.has_lock :%s\r\n",rjsonbuf);
				json_find(injson,"data.open_stay_time",rjsonbuf);
				printf("data.open_stay_time :%s\r\n",rjsonbuf);
				json_find(injson,"data.lock_delay_time",rjsonbuf);
				printf("data.lock_delay_time :%s\r\n",rjsonbuf);
				
				json_add(outjson,"type","Reply_SetConfig");
				json_add(outjson,"data","");
				ostream=json_dumps(outjson, JSON_DECODE_ANY);
				ESP8266_SendNetData(ostream);
			}
			else if(strstr(rjsonbuf,"Request_Command"))
			{
				json_add(outjson,"type","Reply_Command");
				json_add(outjson,"data","");
				ostream=json_dumps(outjson, JSON_DECODE_ANY);
				ESP8266_SendNetData(ostream);
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
void ESP8266_Init(int baudRate)
{
	USART1_Init(baudRate);	 	//串口1初始化为115200
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
	printf("head:%d,rear:%d\r\n",rdQueue.head,rdQueue.rear);
	if(data==NULL)return 1;
	while(ch!=0 && rdQueue.head<rdQueue.rear)
	{
		ch=rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
		*data=ch;
		//printf("data ch:%c\r\n",ch);
		data++;
	}
	*data=0;
	printf("OVER...head:%d,rear:%d\r\n",rdQueue.head,rdQueue.rear);
	return 1;
}

void ESP8266_SendNetData(const char *ostream)
{
	u16 len;
	len=strlen(ostream);
	ESP8266_Sendu8(len>>8);//先发高位
	ESP8266_Sendu8(len);
	ESP8266_printf("%s\r\n",ostream);
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




