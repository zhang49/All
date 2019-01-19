#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "string.h"
#include "esp8266.h"
#include "stdlib.h"
#include "string.h"

int main(void)
{ 
	u8 i,j;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	delay_init();	    	 		//��ʱ������ʼ��	 
	LED_Init();							//��ʼ����LED���ӵ�Ӳ���ӿ�
	USART3_Init(115200);		//����3��ʼ��Ϊ115200
	//��ESP8266sLen��Ϊ0,��ʱ��5ms ����һ�γ���ΪSend_SingleLen�ֽ�
	ESP8266_Init(115200,5);
	printf("start...\r\n");
	while(1)
	{
		ESP8266_RecvProcess();
	}
	//ESP8266_Start();
	//ESP8266_SendNetData("ESP8266 start...\r\n");
	//ESP8266_test();
}



