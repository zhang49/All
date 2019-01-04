#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "string.h"
#include "esp8266.h"
#include "myspi.h"

extern u16 Spi1ReadCnt;
 int main(void)
 { 
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	delay_init();	    	 //延时函数初始化	 
	LED_Init();				//初始化与LED连接的硬件接口
	LED0=0;
	delay_ms(500);
	LED0=1;
	//ESP8266_Init();
	USART3_Init(115200);		//串口3初始化为115200
	Spi1SlaveInit();
	//ESP8266_Start(LOCAL,NULL,NULL,0);
	printf("start...\r\n");
			
	while(1)SPI1_SendData("dsds");
}	



