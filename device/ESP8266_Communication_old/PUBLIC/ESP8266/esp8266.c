#include "esp8266.h"
#include "esp8266device.h"
#include "usart.h"
#include "string.h"
#include "delay.h"
#include <stdarg.h>
#include <jansson.h>
#include "netdef.h"

//extern struct USART2_RD_QUEUE rdQueue;	//队列缓冲区
extern u8 readATFlag;	//读AT指令标记

extern struct SERVER_CLIENT_RECVBUF sRecvBuf[SERVER_ACCEPT_MAX];

u8 sAcceptCount=0;

void ESP8266_Init()
{
	u8 i=0;
	ESP8266_DEVICE_Init(115200);
	USART3_Init(115200);		//串口3初始化为115200
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

u8 ESP8266_Start(enum ESP8266STARTMODE mode,char *ssid,char *psw,int restartflag)
{
//	while(1)
//	{
//		ESP8266_test();
//		delay_ms(1);
//	}
//	return 0;
	u8 i;	
	if(restartflag)ESP8266_SendCmd("AT+CIPCLOSE");
	delay_ms(20);
	for(i=0;i<SERVER_ACCEPT_MAX;i++)
	{
		sRecvBuf[i].rear=0;
		sRecvBuf[i].head=0;
		sRecvBuf[i].singleLen=0xffff;
		sRecvBuf[i].cursor= 0;
	}
	rdQueue.rear=0;
	rdQueue.head=0;
	readATFlag=1;	
	if(restartflag)ESP8266_RST();
	switch(mode)
	{
		case LOCAL:
			if(!ESP8266_LocalMode(APTcpServerPort,ssid,psw))return 0;
			break;
		case CLOUD:
			if(!ESP8266_CLOUDMode("TCP",ssid ,psw ,CLOUD_TCP_ADDRESS,CLOUD_TCP_PORT))return 0;
			break;
	}
	if(!restartflag)ESP8266_RecvProcess(mode);
	return 1;
}
/*
*ESP8266 AP模式下，开启TCP服务器，接收数据
*AT+ CIPSERVER=<mode>[,<port>]  必须AT+CIPMODE=0非透传
*AT+ CIPMUX=1 时才能开启server模式
*/
u8 ESP8266_LocalMode(u32 tcpServerPort,char *ssid,char *psw)
{
	u8 op=0;
	u8 ret;
	u8 count=0;
	char temp[100];
	ESP8266_CloseEcho();
	while(1)
	{
		switch(op)
		{
			case 0:
				count++;
				if(ESP8266_ATTest(20))//测试指令
				 {op++;count=0;}
				break;
			case 1:
				//开STATION模式，方便调试
				count++;
				if(ESP8266_CWMODE_Choice(AP))//选择工作模式(模式变更时会重启)
				 {op++;count=0;}
				break;
			case 2:
				count++;
				if(ESP8266_SendCmdWithCheck("AT+CWDHCP=0,1","OK",NULL,50))//使能DHCP
				{op++;count=0;}
				break;
			case 3:
					//调试为STATION，无法设置
				count++;
				if(ssid!=NULL)
				{
					sprintf(temp,"AT+CWSAP=\"%s\",\"%s\",%d,%d",ssid,psw,APCHL,APECN);
					if(ESP8266_SendCmdWithCheck(temp,"OK",NULL,50))
					{op++;count=0;}
				}else 
					op++;
			break;
			case 4:
				count++;
				//修改SERVER模式，为0才能修改CIPMODE
//				if(ESP8266_SendCmdWithCheck("AT+CIPSERVER=0","OK",NULL,50))
					{op++;count=0;}
			
				 break;
			case 5:
				count++;
				//修改CIPMUX模式，为0才能修改CIPMODE
				if(ESP8266_SendCmdWithCheck("AT+CIPMUX=0","OK",NULL,50))
				 {op++;count=0;}
				 break;
			case 6:
				count++;
				//必须为非透传模式,修改其值，只能在单路模式下
				if(ESP8266_SendCmdWithCheck("AT+CIPMODE=0","OK",NULL,50))
				 {op++;count=0;}
				break;
			case 7:
				count++;
				if(ESP8266_SendCmdWithCheck("AT+CIPMUX=1","OK",NULL,50))//必须为多路模式模式
				 {op++;count=0;}
				break;
			case 8:
				if(ESP8266_CreateTcpServer(tcpServerPort))//开启TCP服务器
				{return 1;}
				break;
		}
		if(count>AT_RESEND_MAX)
		{
			op=0;
			printf("error count is out of AT_RESEND_MAX, ReStart ESP8266 Mode.....\r\n");
			ESP8266_RST();
			//break;
		}
	}
	return 0;
}
/*
*ESP8266 STATION模式下，连上wifi，连接到服务器，接收数据
*/

u8 ESP8266_CLOUDMode(char *type ,char *ssid,char *psw,char *address, int port)
{
	u8 op=0;
	u8 count=0;
	if(ssid==NULL)strcpy(ssid,STATIONSSID);
	if(psw==NULL)strcpy(psw,STATIONPSW);
	ESP8266_CloseEcho();//关闭回显
	while(1)
	{
		switch(op)
		{
			case 0:
				count++;
				if(ESP8266_ATTest(20))//测试指令
				{op++;count=0;}
				break;
			case 1:
				count++;
				if(ESP8266_CWMODE_Choice(STATION))//选择工作模式(模式变更时会重启)
				{op++;count=0;}
				break;
			case 2:
				count++;
				if(ESP8266_JoinAP(ssid,psw))//连接wifi
				{op++;count=0;}
				break;
			case 3:
				count++;
				if(ESP8266_SendCmdWithCheck("AT+CIPMUX=0","OK",NULL,50))//单路连接模式
				{op++;count=0;}
				break;
			case 4:
				count++;
				if(ESP8266_ConnectToServer("TCP",CLOUD_TCP_ADDRESS,CLOUD_TCP_PORT))	//连接到服务器
				{return 1;}
				break;
		}
		if(count>AT_RESEND_MAX)
		{
			op=0;
			printf("error count is out of AT_RESEND_MAX, ReStart ESP8266 Mode.....\r\n");
			ESP8266_RST();
			//break;
		}
	}
	return 0;
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
								//printf("find it,%s\r\n",ret);
								return 1;
							}
							else
							{
								key++;//去掉'.'
								//printf("backkey is:%s\r\n",key);
								if(json_find(json_object_get(injson,k),key,ret))
								{
									//printf(" 2----------find it,%s\r\n",ret);
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
					//break;
					//printf(" 3----------find it,%s\r\n",ret);
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
void ESP8266_RecvProcess(enum ESP8266STARTMODE mode)
{
	u8 clientid;
	u8 i=0;
	json_t *injson,*outjson;
	json_error_t *jerror;
	char *ostream;
	char recvData[255];
	char rjsonbuf[255];
	char temp[3][50];
	u8 restartflag=0;
	int rjsoni;
	delay_ms(100);
	//ESP8266_RST();
	//printf("ESP8266_RST() over\r\n");
	switch(mode)
	{
		case LOCAL:
			while(1)
			{
				clientid=0xff;
				ESP8266_ReadLocalCustom(&clientid,10);
				for(i=0;i<SERVER_ACCEPT_MAX;i++)
				{
					if(sRecvBuf[i].head!=sRecvBuf[i].rear)
					{
						//测试
						//ESP8266_test();
						outjson=json_object();
						printf("Client %d recv:%s\r\n",i,sRecvBuf[i].rData[sRecvBuf[i].head%SCR_SLOT_MAX]);
						injson=json_loads(sRecvBuf[i].rData[sRecvBuf[i].head%SCR_SLOT_MAX], JSON_DECODE_ANY,NULL);
						sRecvBuf[i].head++;
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
									ESP8266_SendNet_unTransparent(i,ostream);
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
								ESP8266_SendNet_unTransparent(i,ostream);
							}
							else if(strstr(rjsonbuf,"Request_ESP8266SetConfig"))
							{
								json_find(injson,"data.mode",rjsonbuf);
								strcpy(temp[0],rjsonbuf);
								printf("data.mode :%s\r\n",rjsonbuf);
								json_find(injson,"data.ssid",rjsonbuf);
								strcpy(temp[1],rjsonbuf);
								printf("data.ssid :%s\r\n",temp[1]);
								json_find(injson,"data.psw",rjsonbuf);
								strcpy(temp[2],rjsonbuf);
								printf("data.psw :%s\r\n",temp[2]);
								//不需要保存至FALSH，ESP8266自带FALSH
//								if(ESP8266_SetAPModeConfig(temp[1],temp[2],5,3))
//								{
//									json_add(outjson,"type","Reply_ESP8266SetConfig");
//									json_add(outjson,"data","");
//								}
								ostream=json_dumps(outjson, JSON_DECODE_ANY);
								ESP8266_SendNet_unTransparent(i,ostream);
								//
								if(strstr(temp[0],"ap"))
								{
									ESP8266_Start(LOCAL,temp[1],temp[2],1);
								}
								else if(strstr(temp[0],"station"))
								{
									ESP8266_Start(CLOUD,temp[1],temp[2],1);
								}
							}
							else if(strstr(rjsonbuf,"Request_ESP8266SetRestore"))
							{
								json_add(outjson,"type","Reply_ESP8266SetRestore");
								json_add(outjson,"data","");
								ostream=json_dumps(outjson, JSON_DECODE_ANY);
								ESP8266_SendNet_unTransparent(i,ostream);
								ESP8266_Start(LOCAL,APSSID,APPSW,1);
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
								ESP8266_SendNet_unTransparent(i,ostream);
							}
							else if(strstr(rjsonbuf,"Request_Command"))
							{
								json_add(outjson,"type","Reply_Command");
								json_add(outjson,"data","");
								ostream=json_dumps(outjson, JSON_DECODE_ANY);
								ESP8266_SendNet_unTransparent(i,ostream);
							}
						}
						free(ostream);
						json_decref(injson);
						json_decref(outjson);
					}
				}
			}
			break;
		case CLOUD:
			while(1)
			{
				if(ESP8266_ReadCloudCustom(recvData,10))
				{
					printf("RecvMessage:%s\r\n",recvData);
				}
				//operation
			}
			break;
	}
}

