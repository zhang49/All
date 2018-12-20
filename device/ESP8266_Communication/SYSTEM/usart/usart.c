#include "sys.h"
#include "usart.h"	  

////加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
//#if 1          
////标准库需要的支持函数                 
//struct __FILE 
//{ 
//	int handle; 

//};    
////定义_sys_exit()以避免使用半主机模式    
//_sys_exit(int x) 
//{ 
//	x = x; 
//} 
////重定义fputc函数 ,输出到 USART3 调试
int fputc(int ch, FILE *f)
{      
	while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
    USART3->DR = (u8) ch;   
	return ch;
}
//#endif 

 
 
#if EN_USART1_RX   //如果使能了接收


#endif	

//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
//接收条数
struct USART2_RD_QUEUE rdQueue;

u16 RX_count=0;       //单条数据接收字节数
u8 readATFlag=0;	//标记为读AT指令返回数据
u16 RX_NetLength=0;

void USART1_Init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟

	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9

	//USART1_RX	  GPIOA.10初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART1, &USART_InitStructure); //初始化串口1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
	USART_Cmd(USART1, ENABLE);                    //使能串口1 
}
void USART3_Init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能USART1，GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	//USART3_TX   GPIOB PIN10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PIN 10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB.10

	//USART3_RX	  GPIOB PIN11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PIN 11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB.10

	//Usart3 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	//USART3 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART3, &USART_InitStructure); //初始化串口3
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口3接受中断
	USART_Cmd(USART3, ENABLE);                    //使能串口3 
}

void USART3_Send(char *data)                	//串口3发送
{
	while(*data!=0)USART3_Putc(*(data++));
}
void USART3_Putc(char ch)              	//串口3发送字符
{
	while((USART3->SR&0X40)==0);//等待发送完毕   
	USART3->DR = (u8) ch; 
}
void USART3_IRQHandler(void)                	//串口3中断服务程序
{
	//u8 recvByte;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{  
		//recvByte =USART_ReceiveData(USART3);
		//USART1_Putc(recvByte);
	} 
}

void USART1_Send(char *data)                	//串口1发送
{
	while(*data!=0)USART1_Putc(*(data++));
}
void USART1_Putc(char ch)              	//串口1发送字符
{
	while((USART1->SR&0X40)==0);//等待发送完毕   
	USART1->DR = (u8) ch; 
}
/*
*接收AT指令时，网络包乱入的情况
*接收网络包前，AT正常接收完毕，RX_count=0，开始接收网络包
*/
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 recvByte;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) 
	{  
		recvByte =USART_ReceiveData(USART1);
		//if(readATFlag) //接收中断(接收到的数据必须是0x0d 0x0a(\r\n)结尾)
		if(readATFlag)
		{
			if(RX_count&0x8000)//已接收了0x0d
			{
				if(recvByte==0x0a && (RX_count&0x7FFF)>0)//接收到0d 0a结尾,且长度大于\r\n
				{
					//收到单条完整数据后改变rdQueue.rear
					rdQueue.rear+=(RX_count&0x7fff);
					rdQueue.data[(++rdQueue.rear)%USART_SLOT_SIZE]=0;
				}
				RX_count=0;
				if(recvByte==0x0d)RX_count|=0x8000;//接收到0d 0d结尾 不接收该数据
//				else	//继续存入
//				{
//					RX_count++;
//					//rdQueue.rear+(RX_count&0x3fff)，先不改变rdQueue.rear，否则数据可能读取不全
//					rdQueue.data[(rdQueue.rear+(RX_count&0x7fff))%USART_SLOT_SIZE]=recvByte;
//				}
			}
			else
			{
				if(recvByte==0x0d)
				{
					RX_count|=0x8000;//接收到的是0x0d
				}
				else
				{
					//+PID,0,15:
					RX_count++;
					if(rdQueue.rear>0xffffff00)//rear即将上溢
					{
						rdQueue.rear-=(rdQueue.head-rdQueue.head%USART_SLOT_SIZE);
						rdQueue.head%=USART_SLOT_SIZE;
					}
					//rdQueue.rear+(RX_count&0x3fff)，先不改变rdQueue.rear，否则数据可能读取不全
					rdQueue.data[(rdQueue.rear+(RX_count&0x7fff))%USART_SLOT_SIZE]=recvByte;
				}
			}
		}
		else
		{
			//收到1个字节改变rdQueue.rear
			rdQueue.data[(++rdQueue.rear)%USART_SLOT_SIZE]=recvByte;
		}
//		else 
//		{	
//			//透传模式下没有+IPD,<len>:前缀，暂不处理
//			if((RX_count&0xc000)==0x4000)//接收了I
//			{
//				if(recvByte=='P')//p
//				{
//					RX_count|=0x8000;//RX_count最高2位 11
//				}
//				else if(recvByte=='D' && (RX_count&0xc000)==0xc000)//D且接收了I P
//				{
//					
//				}
//			
//			}else
//			{
//				if(recvByte=='I')RX_count|=0x4000;//RX_count最高2位 01
//			}
//			//数据不应做过多处理，直接存入缓冲区
//			if(RX_NetLength==0)
//			{
//				RX_count=0;
//				RX_NetLength=recvByte;
//			}
//			else
//			{
//				RX_count++;
//				rdQueue.data[(rdQueue.rear+RX_count)%USART_SLOT_SIZE]=recvByte;
//				if(RX_count==RX_NetLength)//收尾,改变rdQueue.rear
//				{
//					rdQueue.rear+=(RX_count&0x7fff);
//					rdQueue.rear=(rdQueue.rear+1)%USART_SLOT_SIZE;
//					rdQueue.data[rdQueue.rear]=0;
//					RX_NetLength=0;
//				}
//			}
//			
//			if(RX_count&0x8000)//已接收了0x0d
//			{
//				if(recvByte==0x0a && (RX_count&0x7FFF)>0)//接收到0d 0a结尾,且长度大于\r\n
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
//					RX_count|=0x8000;//接收到的是0x0d
//				}
//				else
//				{
//					if(rdQueue.rear>0xffffff00)//rear即将上溢
//					{
//						rdQueue.rear-=(rdQueue.head-rdQueue.head%USART_SLOT_SIZE);
//						rdQueue.head%=USART_SLOT_SIZE;
//					}
//					RX_count++;
//					//rdQueue.rear+(RX_count&0x3fff)，先不改变rdQueue.rear，否则数据可能读取不全
//					rdQueue.data[(rdQueue.rear+(RX_count&0x7fff))%USART_SLOT_SIZE]=recvByte;
//				}
//			}
//		}
	} 
} 




