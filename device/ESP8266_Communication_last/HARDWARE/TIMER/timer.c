#include "timer.h"
#include "led.h"
#include "dma.h"
#include "esp8266.h"

extern char ESP8266sData[];//��Ҫ���͵Ļ�����
extern char sendBuf[];//���ͻ�����
extern u16 ESP8266sLen;//��Ҫ���͵��ܳ���
extern const u8 Send_SingleLen;//���η��͵����ݳ���

u8 curLen = 0;//��ǰ�ѷ��͵����ݳ���

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

void USART_DMA_checkAndSend()
{
	u16 i=0;
	if(curLen == 0 && ESP8266sLen != 0)
	{
		for(i=0;i<Send_SingleLen && i<ESP8266sLen;i++)
				sendBuf[i] = ESP8266sData[curLen + i];
		curLen += i;
		MYDMA_Enable(DMA1_Channel4,i);//��ʼһ��DMA����
		
	}
	if(DMA_GetFlagStatus(DMA1_FLAG_TC4)!=RESET)//�ȴ�ͨ��4�������
	{
		DMA_ClearFlag(DMA1_FLAG_TC4);//���ͨ��4������ɱ�־
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
			MYDMA_Enable(DMA1_Channel4,len);//��ʼһ��DMA���䣡
		}
	}
}


void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
	{
		USART_DMA_checkAndSend();
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM �ж�Դ 
	}
}












