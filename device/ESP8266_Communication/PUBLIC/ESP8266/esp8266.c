#include "esp8266.h"
#include "esp8266device.h"
#include "usart.h"
#include "string.h"
#include "delay.h"
#include <stdarg.h>
#include <jansson.h>
#include "netdef.h"

//extern struct USART2_RD_QUEUE rdQueue;	//���л�����
extern u8 readATFlag;	//��ATָ����
u8 AT_RESEND_MAX=5;		//����ʧ��ʱ������ط�����
u32 APTcpServerPort=7777;	//APģʽ�´����ķ������˿�

extern struct SERVER_CLIENT_RECVBUF sRecvBuf[SERVER_ACCEPT_MAX];

u8 sAcceptCount=0;

void ESP8266_Init()
{
	u8 i=0;
	ESP8266_DEVICE_Init(115200);
	USART3_Init(115200);		//����3��ʼ��Ϊ115200
	for(i=0;i<SERVER_ACCEPT_MAX;i++)
	{
		sRecvBuf[i].rear=0;
		sRecvBuf[i].head=0;
		sRecvBuf[i].singleLen=0xffff;
		sRecvBuf[i].cursor= 0;
	}
	//delay_ms(500);
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
	ESP8266_Sendu8(sdlen>>8);//�ȷ���λ
	ESP8266_Sendu8(sdlen);
	ESP8266_printf("%s",out);
	
	printf("send %d\r\n",(u8)sdlen);
	printf("send %d\r\n",sdlen);
	free(out);
		
	//������json_tʱ�����ü���Ϊ1
	//����json_decref()һ�Σ��������ü��� -1 ��������Ϊ0���Զ��ͷ�����
	json_decref(outjson);
	json_decref(injson);
	
	return;
	
	injson=json_loads(out, JSON_DECODE_ANY,NULL);
	json_find(injson,"data.isopen",ret);
	
	printf("find data.isopen :%s\r\n",ret);

	
	//json_find(injson,"data.version",ret);
	
	//printf("------------------ret is :           %s\r\n",ret);
	
	//�����¸�ֵΪ��һ�����ã���Ҫ�ͷ�ԭ�ȵģ����������ڴ�й©
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

u8 ESP8266_Start(enum ESP8266STARTMODE mode)
{
//	while(1)
//	{
//		ESP8266_test();
//		delay_ms(1);
//	}
//	return 0;
	switch(mode)
	{
		case LOCAL:
			if(ESP8266_LocalMode(APTcpServerPort))
				ESP8266_RecvProcess(LOCAL);
			break;
		case CLOUD:
			if(ESP8266_CLODEMode("TCP",TCP_ADDRESS,TCP_PORT))
				ESP8266_RecvProcess(CLOUD);
			break;
	}
	return 1;
}
/*
*ESP8266 APģʽ�£�����TCP����������������
*AT+ CIPSERVER=<mode>[,<port>]  ����AT+CIPMODE=0��͸��
*AT+ CIPMUX=1 ʱ���ܿ���serverģʽ
*/

u8 ESP8266_LocalMode(u32 tcpServerPort)
{
	u8 op=0;
	u8 count=0;
	ESP8266_CloseEcho();
	while(1)
	{
		switch(op)
		{
			case 0:
				count++;
				if(ESP8266_ATTest(20))//����ָ��
				 {op++;count=0;}
				break;
			case 1:
				//DHCPָ��	��Ч
				//count++;
				op++;
				//if(ESP8266_SendCmdWithCheck("AT+CWDHCP=2,1","OK",NULL,50))//ʹ��DHCP
				//{op++;count=0;}
				break;
			case 2:
				//��STATIONģʽ���������
				count++;
				if(ESP8266_CWMODE_Choice(STATION))//ѡ����ģʽ,(������)
				 {op++;count=0;}
				break;
			case 3:
				count++;
				//�޸�SERVERģʽ��Ϊ0�����޸�CIPMODE
				if(ESP8266_SendCmdWithCheck("AT+CIPSERVER=0","OK",NULL,50))
				 {op++;count=0;}
			case 4:
				count++;
				//�޸�CIPMUXģʽ��Ϊ0�����޸�CIPMODE
				if(ESP8266_SendCmdWithCheck("AT+CIPMUX=0","OK",NULL,50))
				 {op++;count=0;}
			case 5:
				count++;
				//����Ϊ��͸��ģʽ,�޸���ֵ��ֻ���ڵ�·ģʽ��
				if(ESP8266_SendCmdWithCheck("AT+CIPMODE=0","OK",NULL,50))
				 {op++;count=0;}
				break;
			case 6:
				count++;
				if(ESP8266_SendCmdWithCheck("AT+CIPMUX=1","OK",NULL,50))//����Ϊ��·ģʽģʽ
				 {op++;count=0;}
				break;
			case 7:
				if(ESP8266_CreateTcpServer(tcpServerPort))//����TCP������
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
*ESP8266 STATIONģʽ�£�����wifi�����ӵ�����������������
*/

u8 ESP8266_CLODEMode(char *type ,char *address, int port)
{
	u8 op=0;
	u8 count=0;
	ESP8266_CloseEcho();//�رջ���
	while(1)
	{
		switch(op)
		{
			case 0:
				count++;
				if(ESP8266_ATTest(20))//����ָ��
				{op++;count=0;}
				break;
			case 1:
				count++;
				if(ESP8266_CWMODE_Choice(STATION))//ѡ����ģʽ,(������)
				{op++;count=0;}
				break;
			case 2:
				count++;
				if(ESP8266_JoinAP(SSID,PASSWORD))//����wifi
				{op++;count=0;}
				break;
			case 3:
				count++;
				if(ESP8266_SendCmdWithCheck("AT+CIPMUX=0","OK",NULL,50))//��·����ģʽ
				{op++;count=0;}
				break;
			case 4:
				count++;
				if(ESP8266_ConnectToServer("TCP",TCP_ADDRESS,TCP_PORT))	//���ӵ�������
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
			//json_string(value) ���·����ڴ棬����ȡ����Ҫ�ͷ�
			//set_new ��ȡvalue ��valueʱ�½��ģ��ڵ��ú��޷�ʹ��
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
//���� �޷�����json_t ����ʱ
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
								//�����ҵ���
								strcpy(ret,json_string_value(v));
								//printf("find it,%s\r\n",ret);
								return 1;
							}
							else
							{
								key++;//ȥ��'.'
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
		json_t *data;
		//ȡ injson �еĵ�һ����ֵ���ַ�������
		data=json_array_get(injson , 0 ); 
		if (json_is_object (data))
    { 
			data=json_object_get(data ,"type");
			if (json_is_string(data))
			{
				strcpy(type,json_string_value(data));
				json_object_clear(data);
				//printf("GetJsonType type is:%s\r\n",type);
				return 1;
			}
    }
		printf("GetJsonType error\r\n");
		json_decref(data);
		return 0;
}
/*
*��Json����,key or key.key
*/
u8 getJsonData(json_t *injson,char *param,int *retInteger,char *retCharArray)
{
	u8 i=0;
	json_t *data;
	u8 flag=1;
	while(flag)
	{
		char frontp[50];
		flag=0;
		if(strstr(param,"."))//param ����һ��
		{
			u8 flen;
			flen=strlen(param)-strlen(strstr(param,"."));
			strncpy(frontp,param,flen);
			frontp[flen]=0;
			param=strstr(param,".");
			param++;
			//printf("frontp is :%s\r\n",frontp);
			//printf("size is :%d\r\n",json_array_size(injson));
			for(i=0;i<json_array_size(injson);i++)
			{
				data=json_array_get(injson ,i);
				if(json_is_array(data))
				{
					//����Ϊ���飬Ĭ��JsonΪ  { object : array(or object)}
					//printf("%i is array\r\n",i);
					flag=1;
					injson=json_object_get(data,frontp);
					break;
				}
				else if(json_is_object(data))
				{		
					//printf("%i is object\r\n",i);
					data=json_object_get(data,frontp);
					if(json_is_object(data))
					{
						flag=1;
						injson=data;
						//printf("find %s is object\r\n",frontp);
						break;
					}
					/////////
					else if(json_is_array(data))
					{
						flag=1;
						injson=data;
					//	printf("find %s is array\r\n",frontp);
						break;
					}
				}
			}
			if(!flag)
			{
				//�����Ҳ���
				//printf("nofound..\r\n");
				//free(data);
				//json_object_clear(data);
				json_decref(data);
				return 0;
			}
		}
		else
		{
			//param û����һ��
			flag=0;
			strcpy(frontp,param);
			//printf("need find key is:%s\r\n",param);
			if(json_is_array(injson))
			{
				for(i=0;i<json_array_size(injson);i++)
				{
					data=json_array_get(injson,i);
					data=json_object_get(data,param);
					if(data!=NULL)break;					
				}
			}
			else data=json_object_get(injson,param);
			if(json_is_string(data))
			{
				if(retCharArray!=NULL)
				{
					flag=1;
					//printf("finally find is:%s\r\n",json_string_value(data));
					strcpy(retCharArray,json_string_value(data));
					break;
				}
			}
			else if(json_is_integer(data))
			{
				if(retInteger!=NULL)
				{
					flag=1;
					*retInteger=json_integer_value(data);
					//printf("finally find is:%d\r\n",retInteger);
					break;
				}
			}
		}
	}	
	//�˷����ͷ�json_t �ᵼ�³������
	//json_object_clear(data);
	json_decref(data);
	//free(data);
	if(flag)return 1;
	return 0;
}

//����Json����
u8 Send_Json(u8 cid)
{
	json_t *root,*injson,*d1,*d2;
	json_t *arr1 = json_array();
	json_t *arr2 = json_array();
	json_error_t *jerror;
	u8 i=0;
  char *out;
	char data[200];
	int intdd;
	char type[200];
	printf("over\r\n");
	//��Json
	root = json_pack("[{s:s},[s:[{s:s},{s:i}]]]", "type", "requestdata", "data"
	, "id","thisisid","per",23);
  out = json_dumps(root, JSON_DECODE_ANY);
	
	free(out);
	json_decref(root);
	return 0;
}
//��ʼ������������
void ESP8266_RecvProcess(enum ESP8266STARTMODE mode)
{
	u8 clientid;
	u8 i=0;
	json_t *injson;
	json_error_t *jerror;
	char *out;
	char recvData[255];
	char msgtype[100];
	char rjsonbuf[255];
	int rjsoni;
	delay_ms(100);
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
						//����
						ESP8266_test();
						printf("Client %d recv:%s\r\n",i,sRecvBuf[i].rData[sRecvBuf[i].head%SCR_SLOT_MAX]);
						
						sRecvBuf[i].head++;
//						injson=json_loads(sRecvBuf[i].rData[sRecvBuf[i].head%SCR_SLOT_MAX], JSON_DECODE_ANY,NULL);
//						if(GetJsonType(injson,msgtype))
//						{
//							printf("Message Type is:%s\r\n",msgtype);
//							if(json_find(injson,"data.id",rjsonbuf))
//							{
//								printf("getJson data.id is:%s\r\n",rjsonbuf);
//							}
//							else
//							{
//								printf("data.id not found\r\n");
//							}		
//						}
//						else
//						{
//								printf("message type not found\r\n");
//						}
//						json_decref(injson);
					}
				}
				continue;
				if(clientid!=0xff)
				{
					switch(clientid)
					{
						case 0:
							
							while(sRecvBuf[clientid].head!=sRecvBuf[clientid].rear)
							{
//								ESP8266_printf("AT+CIPSEND=0,5\r\n");
//								delay_ms(50);
//								ESP8266_printf("12345");
								//ESP8266_test();
								//printf("Recv Data from Cilent 0 :%s\r\n",(char *)sRecvBuf[clientid].rData[sRecvBuf[clientid].head%SCR_SLOT_MAX]);
							//	printf("Recv Data from Cilent 0 :\r\n");
								//sRecvBuf[clientid].head++;
								//printf("head:%d,rear:%d\r\n",sRecvBuf[clientid].head,sRecvBuf[clientid].rear);
								//��Json
								//out="[{\"type\": \"requestdata\"},[\"data\":[{\"id\": \"thisisid\"},{\"per\": 23}]]]";
								
//								injson=json_loads(sRecvBuf[clientid].rData[sRecvBuf[clientid].head%SCR_SLOT_MAX], JSON_DECODE_ANY,jerror);
//								sRecvBuf[clientid].head++;
//								//sRecvBuf[clientid].head++;
//								//out = json_dumps(injson, JSON_DECODE_ANY);
//								//printf("json to string is:%s\r\n",out);
////								GetJsonType(injson,msgtype);
////								if(strstr(msgtype,"requestdata"))
////								{
////									printf("recv requestdata\r\n");
////								}
//								if(getJsonData(injson,"data.id",NULL,rjsonbuf))
//								{
//									printf("getJson data.id is:%s\r\n",rjsonbuf);
//								}
//								else
//								{
//									printf("data.id not found\r\n");
//								}
//								free(injson);

								//�ѷ���
								//msgtype=(u8)sRecvBuf[clientid].rData[0];
								//msgtype=(msgtype<<8)|(u8)sRecvBuf[clientid].rData[1];
							}
							break;
						case 1:
							printf("  Cilent 1 :");
							break;
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

