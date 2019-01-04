#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "string.h"
#include "esp8266.h"
#include "exti.h"

 int main(void)
 { 
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	delay_init();	    	 //��ʱ������ʼ��	 
	LED_Init();				//��ʼ����LED���ӵ�Ӳ���ӿ�
	LED0=0;
	LED0=1;
	ESP8266_Init(115200);
	EXTIX_Init();
	USART3_Init(115200);		//����3��ʼ��Ϊ115200
	//ESP8266_Start();
	while(1)
	{
		ESP8266_RecvProcess();
	}
}



