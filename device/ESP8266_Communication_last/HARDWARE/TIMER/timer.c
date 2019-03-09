#include "timer.h"
#include "led.h"
#include "dma.h"
#include "esp8266.h"

extern char ESP8266sData[];//需要发送的缓冲区
extern char sendBuf[];//发送缓冲区
extern u16 ESP8266sLen;//需要发送的总长度
extern const u8 Send_SingleLen;//单次发送的数据长度

u8 curLen = 0;//当前已发送的数据长度

//定时器3
//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(  //使能或者失能指定的TIM中断
		TIM3, //TIM2
		TIM_IT_Update ,
		ENABLE  //使能
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //抢占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);  //使能TIMx外设
							 
}

void USART_DMA_checkAndSend()
{
	u16 i=0;
	if(curLen == 0 && ESP8266sLen != 0)
	{
		for(i=0;i<Send_SingleLen && i<ESP8266sLen;i++)
				sendBuf[i] = ESP8266sData[curLen + i];
		curLen += i;
		MYDMA_Enable(DMA1_Channel4,i);//开始一次DMA传输
		
	}
	if(DMA_GetFlagStatus(DMA1_FLAG_TC4)!=RESET)//等待通道4传输完成
	{
		DMA_ClearFlag(DMA1_FLAG_TC4);//清除通道4传输完成标志
		if(curLen == ESP8266sLen)
		{
			ESP8266sLen = 0;
			curLen = 0;
			return;
		}
		else
		{
			u8 len;
			if(ESP8266sLen == 0)return;
			len = Send_SingleLen;
			if(curLen+Send_SingleLen > ESP8266sLen)
			{
				len = ESP8266sLen - curLen;
			}
			for(i=0;i<len;i++)		
				sendBuf[i] = ESP8266sData[curLen + i];
			curLen += len;
			MYDMA_Enable(DMA1_Channel4,len);//开始一次DMA传输！
		}
	}
}


void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
	{
		USART_DMA_checkAndSend();
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
	}
}












