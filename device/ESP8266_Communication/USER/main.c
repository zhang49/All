#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "string.h"
#include "esp8266.h"

 int main(void)
 { 
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	delay_init();	    	 //��ʱ������ʼ��	 
	LED_Init();				//��ʼ����LED���ӵ�Ӳ���ӿ�
	LED0=0;
	delay_ms(500);
	LED0=1;
	ESP8266_Init();
	ESP8266_Start(LOCAL,NULL,NULL);
	while(1) 
	{		 	  			
		delay_ms(500);
	}											    
}	



