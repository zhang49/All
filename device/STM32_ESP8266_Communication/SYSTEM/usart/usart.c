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
enum ComSlaveRecvState m_state;

u8 m_total_check_calc = 0;
u16 m_len = 0;
u16 dataLen = 0;
u8 m_type;
u8 m_total_check;

u16 RX_count=0;       //�������ݽ����ֽ���

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


void process_recv(uint16_t v)
{
	
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
		recvByte = USART_ReceiveData(USART1);
		restart:
		switch(m_state){
			case CSRS_FLAG_1:
					if (recvByte == 'Z'){
							m_state = CSRS_FLAG_2;
							m_total_check_calc += recvByte;
					}
					break;
			case CSRS_FLAG_2:
					if (recvByte == 'Y'){
							m_state = CSRS_LEN_1;
							m_total_check_calc += recvByte;
					}else{
							m_state = CSRS_FLAG_1;
							dataLen=0;
							m_total_check_calc = 0;
							goto restart;
					}
					break;
					
			case CSRS_LEN_1:
					//2λ��ų���
					dataLen+=2;
					m_state = CSRS_LEN_2;
					m_len = recvByte;
					m_len<<=8;
					m_total_check_calc += recvByte;
					break;
			case CSRS_LEN_2:
					m_state = CSRS_TYPE;
					m_len |= recvByte; 
					m_total_check_calc += recvByte;
					break;
			case CSRS_TYPE:
					m_type = recvByte;
					if (1){
						m_state = CSRS_REMARK_ID;
						dataLen++;
						rdQueue.data[(rdQueue.rear+dataLen)%USART_SLOT_SIZE]=recvByte;
						m_total_check_calc += recvByte;
					}else{
							m_state = CSRS_FLAG_1;
							m_total_check_calc = 0;
							dataLen=0;
							goto restart;
					}
					break;
			case CSRS_REMARK_ID:
				m_state = CSRS_DATA;
				dataLen++;
			  rdQueue.data[(rdQueue.rear+dataLen)%USART_SLOT_SIZE]=recvByte;
			  m_total_check_calc += recvByte;
				break;
			
			case CSRS_DATA:
					//����rdQueue
					if(dataLen<m_len-3)
					{
						dataLen++;
						rdQueue.data[(rdQueue.rear+dataLen)%USART_SLOT_SIZE]=recvByte;
						m_total_check_calc += recvByte;
						break;
					}else{
						m_state=CSRS_TOTAL_CHECK;
					}
			case CSRS_TOTAL_CHECK:
					m_total_check = recvByte;
					m_state = CSRS_FLAG_1;
					//printf("recvTotalCheck is:%d,calc TotalCheck is:%d\r\n",m_total_check,m_total_check_calc);
					if (m_total_check == m_total_check_calc){
						//�ɹ����յ���������
						//ramarkID(1) type(1) Datalength(2) data(...)
						rdQueue.data[(rdQueue.rear + 1)%USART_SLOT_SIZE]=dataLen>>8;
						rdQueue.data[(rdQueue.rear + 2)%USART_SLOT_SIZE]=dataLen&0x00ff;
						rdQueue.rear+=dataLen;
						//printf("rear is:%d,\tdataLen is:%d,\tdata is:%s\r\n",rdQueue.rear,dataLen,rdQueue.data);
						m_total_check_calc = 0;
						dataLen = 0;
						m_len = 0;
					}else{
							m_total_check_calc = 0;
							dataLen=0;
							goto restart;
					}
					
					break;
			default:
					break;
		}
		
	}
} 




