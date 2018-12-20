#include "sys.h"
#include "usart.h"	  

////�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
//#if 1          
////��׼����Ҫ��֧�ֺ���                 
//struct __FILE 
//{ 
//	int handle; 

//};    
////����_sys_exit()�Ա���ʹ�ð�����ģʽ    
//_sys_exit(int x) 
//{ 
//	x = x; 
//} 
////�ض���fputc���� ,����� USART3 ����
int fputc(int ch, FILE *f)
{      
	while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
    USART3->DR = (u8) ch;   
	return ch;
}
//#endif 

 
 
#if EN_USART1_RX   //���ʹ���˽���


#endif	

//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
//��������
struct USART2_RD_QUEUE rdQueue;

u16 RX_count=0;       //�������ݽ����ֽ���
u8 readATFlag=0;	//���Ϊ��ATָ�������
u16 RX_NetLength=0;

void USART1_Init(u32 bound)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��

	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9

	//USART1_RX	  GPIOA.10��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	//USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
	USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 
}
void USART3_Init(u32 bound)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��USART1��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	//USART3_TX   GPIOB PIN10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PIN 10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB.10

	//USART3_RX	  GPIOB PIN11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PIN 11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB.10

	//Usart3 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	//USART3 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART3, &USART_InitStructure); //��ʼ������3
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//��������3�����ж�
	USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���3 
}

void USART3_Send(char *data)                	//����3����
{
	while(*data!=0)USART3_Putc(*(data++));
}
void USART3_Putc(char ch)              	//����3�����ַ�
{
	while((USART3->SR&0X40)==0);//�ȴ��������   
	USART3->DR = (u8) ch; 
}
void USART3_IRQHandler(void)                	//����3�жϷ������
{
	//u8 recvByte;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{  
		//recvByte =USART_ReceiveData(USART3);
		//USART1_Putc(recvByte);
	} 
}

void USART1_Send(char *data)                	//����1����
{
	while(*data!=0)USART1_Putc(*(data++));
}
void USART1_Putc(char ch)              	//����1�����ַ�
{
	while((USART1->SR&0X40)==0);//�ȴ��������   
	USART1->DR = (u8) ch; 
}
/*
*����ATָ��ʱ���������������
*���������ǰ��AT����������ϣ�RX_count=0����ʼ���������
*/
void USART1_IRQHandler(void)                	//����1�жϷ������
{
	u8 recvByte;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) 
	{  
		recvByte =USART_ReceiveData(USART1);
		//if(readATFlag) //�����ж�(���յ������ݱ�����0x0d 0x0a(\r\n)��β)
		if(readATFlag)
		{
			if(RX_count&0x8000)//�ѽ�����0x0d
			{
				if(recvByte==0x0a && (RX_count&0x7FFF)>0)//���յ�0d 0a��β,�ҳ��ȴ���\r\n
				{
					//�յ������������ݺ�ı�rdQueue.rear
					rdQueue.rear+=(RX_count&0x7fff);
					rdQueue.data[(++rdQueue.rear)%USART_SLOT_SIZE]=0;
				}
				RX_count=0;
				if(recvByte==0x0d)RX_count|=0x8000;//���յ�0d 0d��β �����ո�����
//				else	//��������
//				{
//					RX_count++;
//					//rdQueue.rear+(RX_count&0x3fff)���Ȳ��ı�rdQueue.rear���������ݿ��ܶ�ȡ��ȫ
//					rdQueue.data[(rdQueue.rear+(RX_count&0x7fff))%USART_SLOT_SIZE]=recvByte;
//				}
			}
			else
			{
				if(recvByte==0x0d)
				{
					RX_count|=0x8000;//���յ�����0x0d
				}
				else
				{
					//+PID,0,15:
					RX_count++;
					if(rdQueue.rear>0xffffff00)//rear��������
					{
						rdQueue.rear-=(rdQueue.head-rdQueue.head%USART_SLOT_SIZE);
						rdQueue.head%=USART_SLOT_SIZE;
					}
					//rdQueue.rear+(RX_count&0x3fff)���Ȳ��ı�rdQueue.rear���������ݿ��ܶ�ȡ��ȫ
					rdQueue.data[(rdQueue.rear+(RX_count&0x7fff))%USART_SLOT_SIZE]=recvByte;
				}
			}
		}
		else
		{
			//�յ�1���ֽڸı�rdQueue.rear
			rdQueue.data[(++rdQueue.rear)%USART_SLOT_SIZE]=recvByte;
		}
//		else 
//		{	
//			//͸��ģʽ��û��+IPD,<len>:ǰ׺���ݲ�����
//			if((RX_count&0xc000)==0x4000)//������I
//			{
//				if(recvByte=='P')//p
//				{
//					RX_count|=0x8000;//RX_count���2λ 11
//				}
//				else if(recvByte=='D' && (RX_count&0xc000)==0xc000)//D�ҽ�����I P
//				{
//					
//				}
//			
//			}else
//			{
//				if(recvByte=='I')RX_count|=0x4000;//RX_count���2λ 01
//			}
//			//���ݲ�Ӧ�����ദ��ֱ�Ӵ��뻺����
//			if(RX_NetLength==0)
//			{
//				RX_count=0;
//				RX_NetLength=recvByte;
//			}
//			else
//			{
//				RX_count++;
//				rdQueue.data[(rdQueue.rear+RX_count)%USART_SLOT_SIZE]=recvByte;
//				if(RX_count==RX_NetLength)//��β,�ı�rdQueue.rear
//				{
//					rdQueue.rear+=(RX_count&0x7fff);
//					rdQueue.rear=(rdQueue.rear+1)%USART_SLOT_SIZE;
//					rdQueue.data[rdQueue.rear]=0;
//					RX_NetLength=0;
//				}
//			}
//			
//			if(RX_count&0x8000)//�ѽ�����0x0d
//			{
//				if(recvByte==0x0a && (RX_count&0x7FFF)>0)//���յ�0d 0a��β,�ҳ��ȴ���\r\n
//				{
//					//
//					rdQueue.rear+=(RX_count&0x7fff);
//					rdQueue.rear=(rdQueue.rear+1)%USART_SLOT_SIZE;
//					rdQueue.data[rdQueue.rear]=0;
//				}
//				RX_count=0;
//				if(recvByte==0x0d)RX_count|=0x8000;
//			}
//			else
//			{
//				if(recvByte==0x0d)
//				{
//					RX_count|=0x8000;//���յ�����0x0d
//				}
//				else
//				{
//					if(rdQueue.rear>0xffffff00)//rear��������
//					{
//						rdQueue.rear-=(rdQueue.head-rdQueue.head%USART_SLOT_SIZE);
//						rdQueue.head%=USART_SLOT_SIZE;
//					}
//					RX_count++;
//					//rdQueue.rear+(RX_count&0x3fff)���Ȳ��ı�rdQueue.rear���������ݿ��ܶ�ȡ��ȫ
//					rdQueue.data[(rdQueue.rear+(RX_count&0x7fff))%USART_SLOT_SIZE]=recvByte;
//				}
//			}
//		}
	} 
} 




