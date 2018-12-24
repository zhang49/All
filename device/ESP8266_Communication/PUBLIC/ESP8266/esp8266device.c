#include "esp8266device.h"
#include "usart.h"
#include "string.h"
#include "delay.h"
#include <stdarg.h>
#include <stdint.h>
#include "led.h"

extern struct USART2_RD_QUEUE rdQueue;	//���л�����
extern u8 readATFlag;	//��ATָ����
extern u8 sAcceptCount;
const u8 acceptmax;
struct SERVER_CLIENT_RECVBUF sRecvBuf[SERVER_ACCEPT_MAX];
u8 readforsend=0;
//��2λ��Cid����14λ�泤��
u16 needReadIpdLength=0;
void ESP8266_DEVICE_Init(int baudRate)
{
	USART1_Init(baudRate);	 	//����1��ʼ��Ϊ115200
	rdQueue.rear=0;
	rdQueue.head=0;
	readATFlag=1;
}
/////////////AT+UART=115200,8,1,0,3���ò����ʣ�����
//�رջ���
u8 ESP8266_CloseEcho(void)
{
	if(ESP8266_SendCmdWithCheck("ATE0","OK",NULL,50))
	{
		//δ�رջ���ǰ����ATE0 OK,���������ε�ATE0
	}
	else{
		delay_ms(10);
		
	
	printf("rdQueue.head:%d,rdQueue.rear:%d\r\n",rdQueue.head,rdQueue.rear);
		ESP8266_ReadATRet(NULL,1);
	}
	return 1;
}
//����ģ��
u8 ESP8266_RST()
{
	//USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//�رմ���1�����ж�,��������������յ�����
	ESP8266_SendCmd("AT+RST");
	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//��������1�����ж�
	while(!ESP8266_CmdIsSuccess("ready",2));
	ESP8266_CloseEcho();
	if(ESP8266_SendCmdWithCheck("AT","OK",NULL,50))
	{
		return 1;
	}
	return 0;
}
//����ָ��  ����data+\r\n
//ret1,re2ΪԤ�ڽ��յ������ݣ�timeoutΪ��ȴ�ʱ��
u8 ESP8266_SendCmdWithCheck(char *data,char *ret1,char *ret2,u16 tenMsTimes)
{
	ESP8266_SendCmd(data);
	return ESP8266_ExpectRet(ret1,ret2,tenMsTimes);
}
u8 ESP8266_SendCmd(char *data)
{
	ESP8266_printf("%s\r\n",data);
}
u8 ESP8266_ATTest(u16 tenMsTimes)
{
	return ESP8266_SendCmdWithCheck("AT","OK",NULL,tenMsTimes);
}
//ѡ����ģʽ
u8 ESP8266_CWMODE_Choice(enum CWMODE mode)
{
	char expectRet[15];
	sprintf(expectRet,"+CWMODE:%d",mode);
	if(ESP8266_SendCmdWithCheck("AT+CWMODE?",expectRet,"OK",20))return 1;
	ESP8266_printf("AT+CWMODE=%d\r\n",mode);
	if(ESP8266_ExpectRet("OK",NULL,20))
	{
		if(ESP8266_RST())return 1;
	}
	return 0;
}
//����APģʽ�µ���Ϣ
u8 ESP8266_SetAPModeConfig(char *ssid,char *psw,int chl,int ecn)
{
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//�رմ���1�����ж�,��������������յ�����
	delay_ms(20);
	readATFlag=1;
	rdQueue.head=0;
	rdQueue.rear=0;
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//��������1�����ж�
	ESP8266_printf("AT+CWSAP=\"%s\",\"%s\",%d,%d",ssid,psw,chl,ecn);
	if(ESP8266_ExpectRet("OK",NULL,50))
	{
		readATFlag=0;
		rdQueue.head=0;
		rdQueue.rear=0;
		return 1;
	}
	readATFlag=0;
	rdQueue.head=0;
	rdQueue.rear=0;
	return 0;
}
//��ԭAPģʽ�µ���Ϣ
u8 ESP8266_RestoreAPConfig()
{
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//�رմ���1�����ж�,��������������յ�����
	delay_ms(20);
	readATFlag=1;
	rdQueue.head=0;
	rdQueue.rear=0;
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//��������1�����ж�
	return 1;
}
//����wifi
u8 ESP8266_JoinAP(char *ssid,char *psw)
{
	char expectRet[50];
	char recvData[100];
	sprintf(expectRet,"+CWJAP:\"%s\"",ssid);
	ESP8266_SendCmd("AT+CWJAP?");
	if(ESP8266_ReadATRet(recvData,255))
	{
		if(strstr(recvData,expectRet))
		{
			ESP8266_ReadATRet(recvData,255);
			return 1;
		}
		if(strstr("No AP",recvData))ESP8266_ReadATRet(recvData,255);
		else //������������WIFI��
		{
			//�˳�����
			if(!ESP8266_SendCmdWithCheck("AT+CWQAP","OK","WIFI DISCONNECT",20))
				return 0;
		}
		ESP8266_printf("AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,psw);
		//������Щ��
		//OK
		//+CWJAP:		FAIL
		if(ESP8266_ExpectRet("WIFI CONNECTED",NULL,1000))
		{
			ESP8266_ReadATRet(recvData,255);//WIFI GOT IP
			ESP8266_ReadATRet(recvData,255);//OK
			return 1;//��������wifi
		}
		else ESP8266_ReadATRet(recvData,255);//��ȡFAIL
	}
	
	return 0;
}
//���ӵ�������,͸��ģʽ
u8 ESP8266_ConnectToServer(char *type,char *ipaddress,int port)
{
	//������͸��ģʽ
	if(!ESP8266_SendCmdWithCheck("AT+CIPMODE=1","OK",NULL,80))return 0;
	ESP8266_printf("AT+CIPSTART=\"%s\",\"%s\",%d\r\n",type,ipaddress,port);
	delay_ms(200);
	//����ִ�жϿ�����
	if(ESP8266_ExpectRet("CONNECT","OK",80))
	{
		if(ESP8266_SendCmdWithCheck("AT+CIPSEND","OK",NULL,80))
		{
			rdQueue.head=0;
			rdQueue.rear=0;
			readATFlag=0;
			delay_ms(100);
			return 1;
		}
		else return 0;
	}
	else return 0;
}
//����������
u8 ESP8266_CreateTcpServer(u32 tcpServerPort)
{
	ESP8266_printf("AT+CIPSERVER=1,%d\r\n",tcpServerPort);
	if(ESP8266_ExpectRet("OK",NULL,50))
	{
		readATFlag=0;
		rdQueue.head=0;
		rdQueue.rear=0;
		return 1;
	}
	else return 0;
}
//���ָ������
//para1ΪNULLʱ�����2�Σ���para2�ķ���ֵΪ׼
//��ʱ���ʱ��Ϊ timeout*10 ms
u8 ESP8266_ExpectRet(char *para1,char *para2,u16 tenMsTimes)
{
	if(para1!=0 && para2==0)
	{
		int ret =ESP8266_CmdIsSuccess(para1,tenMsTimes);
		return ret;
	}
	else if(para1!=0 && para2!=0)
	{
		return ESP8266_CmdIsSuccess(para1,tenMsTimes) & ESP8266_CmdIsSuccess(para2,tenMsTimes);
	}
	else if(para2!=0)
	{
		return ESP8266_CmdIsSuccess(para1,tenMsTimes) | ESP8266_CmdIsSuccess(para2,tenMsTimes);
	}
	return 0;
}
//���ָ���Ƿ��ͳɹ�
u8 ESP8266_CmdIsSuccess(char *expect,u16 tenMsTimes)
{
	char recv[100];
	u8 tc=0;
	printf("Expect:%s\r\n",expect);
	if(!ESP8266_ReadATRet(recv,tenMsTimes))return 0;
	printf("Recv:%s\r\n",recv);
	
	//���Ƚϳ���
	//if(strlen(recv)==strlen(expect) && strstr(recv,expect))
	if(strstr(recv,expect))
	{
		return 1;
	}
	if(strlen(recv)==strlen("busy p...") && strstr(recv,"busy p..."))
	{
		while(rdQueue.head==rdQueue.rear && tc<500)//�ȴ�����,��ʱ5S
		{
			delay_ms(100);
			tc+=1;
		}
		if(rdQueue.head==rdQueue.rear)return 0;//��ʱ������ȴ�busy p...����Ȼû������
		else
		{
			ESP8266_ReadATRet(recv,tenMsTimes);
			printf("After busy p ... Recv:%s\r\n",recv);
			//if(strlen(recv)==strlen(expect) && strstr(recv,expect))return 1;
			if(strstr(recv,expect))return 1;
		}
	}
	return 0;
}

//ȡATָ��ķ����ַ���
u8 ESP8266_ReadATRet(char data[],u16 tenMsTimes)
{
	u8 ch=1;
	int tc=0;
	//printf("tc:%d,timeout:%d\r\n",tc,tenMsTimes);
	while(rdQueue.head==rdQueue.rear && tc<tenMsTimes)//�ȴ�����
	{
		delay_ms(10);
		tc+=1;
	}
	if(rdQueue.head==rdQueue.rear)return 0;//��ʱ�����û������
	//���ݳ��ȳ���100
	if(rdQueue.rear>=rdQueue.head+98)
	{
		rdQueue.head=rdQueue.rear;
		return 0;
	}
	//printf("____tc:%d,timeout:%d\r\n",tc,tenMsTimes);
	//printf("head:%d,rear:%d\r\n",rdQueue.head,rdQueue.rear);
	if(data==NULL)return 1;
	while(ch!=0)
	{
		ch=rdQueue.data[(++rdQueue.head)%USART_SLOT_SIZE];
		*data=ch;
		//printf("data ch:%c\r\n",ch);
		data++;
	}
	*data=0;
	//printf("read over data:%s\r\n",data);
	//printf("read over head:%d,rear:%d\r\n",rdQueue.head,rdQueue.rear);
	return 1;
}
u8 ESP8266_ReadIpdData()
{
	u8 cid;
	u32 cursor=rdQueue.head;
	//printf("begin rdQueue head is:%d,rear is:%d\r\n",rdQueue.head,rdQueue.rear);
	//delay_ms(10);
	if((needReadIpdLength&0x3fff)==0)
	{
		u8 i=0;
		char temp[15];
		u8 cid;
		cursor=rdQueue.head+1;
		for (i = 0; i < 12 && (cursor + i < rdQueue.rear); i++)
			*(temp + i) = rdQueue.data[(cursor + i) % USART_SLOT_SIZE];
		//printf("temp:%s\r\n",temp);
		//��
		if (strstr(temp, "\r\n+IPD,"))
		{
			cursor+=2;
		}
		if (strstr(temp, "+IPD,"))
		{
			u16 len=0;
			cursor += 4;//ָ��','
			//ClientId
			cid = rdQueue.data[++cursor % USART_SLOT_SIZE]-'0';
			printf("cid is:%d\r\n",cid);
			needReadIpdLength=cid;
			//��2λ��Cid����14λ�泤��
			needReadIpdLength<<=14;
			cursor += 1;//ָ��','
			i = 0;
			//+pid
			while (++cursor<=rdQueue.rear && rdQueue.data[(cursor) % USART_SLOT_SIZE] != ':')
			{
				LED0=~LED0;
				len *= 10;
				len += (rdQueue.data[(cursor) % USART_SLOT_SIZE] - '0');
			}
			if(rdQueue.data[(cursor) % USART_SLOT_SIZE] == ':')
			{
				needReadIpdLength|=len;
				printf("+ipd length is:%d,nRIL is:%d\r\n",len,needReadIpdLength);
				rdQueue.head=cursor;
			}
			else return 1;
		}
		else return 0;
	}
	cid=needReadIpdLength&0xc000;
	//printf("Cid is:%d\r\n",cid);
	//printf("needReadIpdLength is:%d\r\n",needReadIpdLength&0x3fff);
	while(rdQueue.rear!=cursor && needReadIpdLength&0x3fff)
	{
				LED0=~LED0;
		//�������ݳ���
		if (sRecvBuf[cid].singleLen == 0xffff)
		{
			//2�ֽڴ�С
			//printf("singleLength is zero\r\n");
			if(cursor+2>rdQueue.rear)break;
			sRecvBuf[cid].singleLen = rdQueue.data[(++cursor) % USART_SLOT_SIZE];
			sRecvBuf[cid].singleLen=(sRecvBuf[cid].singleLen<<8)|rdQueue.data[(++cursor) % USART_SLOT_SIZE];
			needReadIpdLength-=2;
			//printf("singleLength:%d\r\n",sRecvBuf[cid].singleLen);
			//printf("over singleLen rdQueue head:%d,cursor:%d,rear:%d\r\n",rdQueue.head,cursor,rdQueue.rear);
		}
		else
		{
			//put data to recvbuf
			u8 srbcur;
			//printf("singleLength is not zero\r\n");
			//printf("get sRecvBuf[%d].singleLen:%d\r\n",cid,sRecvBuf[cid].singleLen);
			srbcur=sRecvBuf[cid].rear%SCR_SLOT_MAX;
			while (rdQueue.rear!=cursor && needReadIpdLength&0x3fff && sRecvBuf[cid].singleLen > 0)
			{
				LED0=~LED0;
				cursor++;
				sRecvBuf[cid].rData[srbcur][sRecvBuf[cid].cursor++] 
					= rdQueue.data[cursor % USART_SLOT_SIZE];
				sRecvBuf[cid].singleLen--;
				needReadIpdLength--;
			}
			//�������ݽ������
			if (sRecvBuf[cid].singleLen == 0)
			{
				sRecvBuf[cid].rData[srbcur][sRecvBuf[cid].cursor++]=0;
				//printf("recv SingleData is:%s\r\n",(char *)sRecvBuf[cid].rData[sRecvBuf[cid].rear%SCR_SLOT_MAX]);		
				if(sRecvBuf[cid].rear==0xfe)
				{
					sRecvBuf[cid].rear-=(sRecvBuf[cid].head-sRecvBuf[cid].head%SCR_SLOT_MAX);
					sRecvBuf[cid].head%=SCR_SLOT_MAX;
				}
				//ʹ��ͬһ��ֵ sRecvBuf[cid].rear �ĵط���ֹһ��������ʹ��ǰ����
				sRecvBuf[cid].rear++;
				sRecvBuf[cid].cursor = 0;
				sRecvBuf[cid].singleLen = 0xffff;
			}
			//rdQueue.head += (cursor-rdQueue.head);
		}
	}
	rdQueue.head=cursor;
	//printf("over rdQueue head is:%d,rear is:%d\r\n",rdQueue.head,rdQueue.rear);
	return 1;
}
//ESP8266��ΪTCP���������յ�������
//��·ģʽ���յ�����Ϣ��+IPD,ǰ׺
u8 ESP8266_ReadLocalCustom(u8 *id,u16 timeout)
{
	uint8_t cid;
	char temp[15] = {0};
	uint16_t i = 0;
	if(rdQueue.head==rdQueue.rear)return 0;
	if(rdQueue.head>=rdQueue.rear)
	{
		printf("READ ERROR!\r\n");
		return 0;
	}
	LED0=~LED0;
	//printf("begin rdQueue head is:%d,rear is:%d\r\n",rdQueue.head,rdQueue.rear);
	if(!ESP8266_ReadIpdData())
	{
		u16 cursor = rdQueue.head;
		do
		{
			if(rdQueue.data[(cursor+1) % USART_SLOT_SIZE]=='>')
			{
				readforsend=1;
				cursor++;
				break;
			}
			else if(rdQueue.rear<rdQueue.head+5)return 0;
			for (i = 1; cursor+i<rdQueue.rear && rdQueue.data[(cursor+i) % USART_SLOT_SIZE]!='\r' ; i++)
				*(temp + i - 1) = rdQueue.data[(cursor+i) % USART_SLOT_SIZE];
			
			//printf("AT cmd:%s\r\n",temp);
			if(rdQueue.data[(cursor+i+1) % USART_SLOT_SIZE]!='\n')
				return 0;
			i++;//	\r\n
			cursor+=i;
			if (strstr(temp, ",CONNECT F"))
			{
				//printf("Client Connect Fall\r\n");
				cid = rdQueue.data[(rdQueue.head + 1) % USART_SLOT_SIZE]-'0';
				sAcceptCount--;
				break;
			}
			if (strstr(temp, ",CLOSED"))
			{
				//printf("Client Closed\r\n");
				cid = rdQueue.data[(rdQueue.head + 1) % USART_SLOT_SIZE]-'0';
				sAcceptCount--;
				break;
			}
			if (strstr(temp, ",CONNECT"))
			{
				//printf("Client Connect\r\n");
				cid = rdQueue.data[(rdQueue.head + 1) % USART_SLOT_SIZE]-'0';
				sAcceptCount++;
				break;
			}
			if(strstr(temp, "SEND OK"))
			{
				break;
			}
			if(strstr(temp, "SEND") && strstr(temp, "BYTE"))
			{
				break;
			}
			if(strstr(temp, "busy s..."))
			{
				break;
			}
			if(rdQueue.data[(cursor) % USART_SLOT_SIZE]!='\n')cursor-=i;
		}while(0);
		rdQueue.head=cursor;
	}
	//rear��������
	//��usart���޸�rear��head��Σ�գ�������cursorʹ��head��(��)ֵ��
	//usart���޸ģ��˷����п��ܻᵼ��head>rear
	if(rdQueue.rear>0xfffff000)
	{
		rdQueue.rear=rdQueue.rear-(rdQueue.head-rdQueue.head%USART_SLOT_SIZE);
		rdQueue.head%=USART_SLOT_SIZE;
	}
	printf("over rdQueue head is:%d,rear is:%d\r\n",rdQueue.head,rdQueue.rear);
	return 0;
}
/*
*ESP8266��ΪTCP�Ͷ˽��յ������������
*��һ���ֽ�Ϊ���ݳ���
*/
u8 ESP8266_ReadCloudCustom(char data[],u16 timeout)
{
	u16 tc=0;
	u16 len;
	//printf("tc:%d,timeout:%d\r\n",tc,timeout);
	while(rdQueue.head==rdQueue.rear && tc<timeout)//�ȴ�����
	{
		tc+=1;
	}
	if(rdQueue.head==rdQueue.rear)
	{	
		return 0;//��ʱ�����û������
	}
	len=rdQueue.data[(rdQueue.head+1)%USART_SLOT_SIZE];
	//printf("length:%d\r\n",len);
	if(len!=0 && len<=(rdQueue.rear-(rdQueue.head+1)))//ȡ�����������ȵ�����
	{
		u16 i;
		//printf("Recv a full package:%d\r\n",len);
		//ȡ����  rdQueue.data[(rdQueue.head+1)%USART_SLOT_SIZE]���ȴ�С
		rdQueue.head++;	// ���ݿ�ʼλ��
		i=0;
		while(i<len)
		{
			i++;
			*data=rdQueue.data[((rdQueue.head+i)%USART_SLOT_SIZE)];
			//printf("data:%c",*data+'0');
			data++;
		}
		*data=0;
		rdQueue.head=rdQueue.head+len;
		//printf("over\r\n");
		return 1;
	}
	//printf("____tc:%d,timeout:%d\r\n",tc,timeout);
	//printf("head:%d,rear:%d\r\n",rdQueue.head,rdQueue.rear);
	return 0;
}

void ESP8266_SendNet_unTransparent(u8 cid,const char *ostream)
{
	u16 len;
	len=strlen(ostream);
	ESP8266_printf("AT+CIPSEND=%d,%d\r\n",cid,len+2);
	delay_ms(50);
	ESP8266_Sendu8(len>>8);//�ȷ���λ
	ESP8266_Sendu8(len);
	ESP8266_printf("%s",ostream);
}
/*
* %d ���͵��� u32 ��ת���ɶ�Ӧ���ַ����ͣ�����
*/
void ESP8266_printf (char * Data, ... )
{
	const char *s;
	u32 d;   
	va_list arg_ptr;
	va_start(arg_ptr, Data);//ʹ��ָ��arg_ptr��������ջ���еĲ����б�
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
				s = va_arg(arg_ptr, const char *);	//ȡ���ݣ�����Ϊconst char*
				for ( ; *s; s++) 
				{
					ESP8266_Sendu8(*s);
				}
				Data++;
				break;
				case 'd':										//%d	ʮ���Ƶ���������
				d = va_arg(arg_ptr, u32);
				if(d==0)ESP8266_Sendu8('0');
					else ESP8266_senditoa(d);
				Data++;
				break;
				default:	// %s %d���ռλ������������!!!
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

