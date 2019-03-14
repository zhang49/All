#include "timer.h"
#include "led.h"
#include "dma.h"
#include "esp8266.h"

extern char SendBuf[1024];//��Ҫ���͵�����
extern char ESP8266sData[255];//DMA����ͨ��������
extern u16 SendCursor;//��ǰ����λ

//��ʱ��3
//ͨ�ö�ʱ���жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(  //ʹ�ܻ���ʧ��ָ����TIM�ж�
		TIM3, //TIM2
		TIM_IT_Update ,
		ENABLE  //ʹ��
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����
							 
}

u8 DMA_SendData()
{
	if(SendCursor!=0xffff)
	{
		u8 i=0;
		u16 len = 0;
	  len=*(SendBuf+2);
		len<<=8;
		len|=*(SendBuf+3);
		for(;i<(u8)DMA_SINGLE_LEN_MAX && SendCursor<len;i++)*(ESP8266sData+i)=*(SendBuf+(SendCursor++));
		MYDMA_Enable(DMA1_Channel4,i);//��ʼһ��DMA����
		
		//*(ESP8266sData+i)=0;
		//printf("\r\n\t\tDMA SEND DATA TO NET:\r\n%s\r\n",ESP8266sData);
		if(SendCursor>=len)
		{
			SendCursor = 0xffff;
			SendBuf[0] = 0;
		}
		return 1;
	}
	return 0;
}
u16 send_status_tick;
void TIM3_IRQHandler(void)   //TIM3�ж�
{
	send_status_tick++;
	if(send_status_tick>=1){
		ESP8266_send_syn_status();
	}
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
	{
		//USART_DMA_checkAndSend();
		DMA_SendData();
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM �ж�Դ 
	}
}












