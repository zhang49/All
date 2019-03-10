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
	char data[1024]="";
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	delay_init();	    	 		//延时函数初始化	 
	LED_Init();							//初始化与LED连接的硬件接口
	USART3_Init(115200);		//串口3初始化为115200
	ESP8266_Init(115200,200);
	printf("start...\r\n");
	ESP8266_test();
	while(1)
	{
		ESP8266_RecvProcess();
		//if(ESP8266_NeedSendData()==0)ESP8266_SendNetData(data, 1, strlen(data));
	}
	//ESP8266_Start();
	//ESP8266_SendNetData("ESP8266 start...\r\n");
	//ESP8266_test();
}



