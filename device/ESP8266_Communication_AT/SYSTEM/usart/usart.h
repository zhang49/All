#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��
#define USART_SLOT_SIZE  		3072 	//ѭ�����л�������С
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
#define EN_USART3_RX 			1		//ʹ�ܣ�1��/��ֹ��0������3����


struct USART2_RD_QUEUE{
	u8 data[USART_SLOT_SIZE];
	u32 head;
	u32 rear;
};

extern struct USART2_RD_QUEUE rdQueue;	//���л�����
extern u8 readATFlag;
void USART1_Init(u32 bound);
void USART3_Init(u32 bound);
void USART1_Putc(char ch);
void USART1_Send(char *data);
void USART3_Putc(char ch);
void USART3_Send(char *data);

#endif


