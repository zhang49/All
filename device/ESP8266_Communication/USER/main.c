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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	delay_init();	    	 		//延时函数初始化	 
	LED_Init();							//初始化与LED连接的硬件接口
	USART3_Init(115200);		//串口3初始化为115200
	//当ESP8266sLen不为0,定时器5ms 发送一次长度为Send_SingleLen字节
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



