/*****************************************************
* CAN ����źŵı�ʾ
* 1:���Ե�ƽ H2.5v - L2.5v = 0v
* 0:���Ե�ƽ H3.5v - L1.5v = 2v
*****************************************************/ 
#include "bsp_can.h"
#include <stdio.h>

#define CAN_STD (1)

volatile unsigned char can_flag;

/*����*/
CanTxMsg TxMessage = {0}; // ���ͻ�����
CanRxMsg RxMessage = {0}; // ���ջ�����

/* ����CAN1�����ж� */
static void CAN_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure = {0}; 
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	/* Enable the CANy Interrupt */ 
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
/**
 * CAN ģʽ����
 */
static void CAN_Mode_Config(void)
{
	CAN_InitTypeDef CAN_InitStructure = {0};
	
	/* CAN1 param config */
	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);
	
	/* CAN1 unit config */
	CAN_InitStructure.CAN_TTCM = DISABLE;	//MCR-TTCM �ر�ʱ�䴥��ͨ��ģʽʹ��
	CAN_InitStructure.CAN_ABOM = ENABLE; 	//MCR-ABOM �Զ����߹���
	CAN_InitStructure.CAN_AWUM = ENABLE; 	//MCR-AWUM ʹ���Զ�����
	CAN_InitStructure.CAN_NART = DISABLE; //MCR-NART ��ֹ�Զ��ش�
	CAN_InitStructure.CAN_RFLM = DISABLE; //MCR-RFLM ����FIFO����ģʽ DISABLE-���ʱ�±��ĻḲ��ԭ�б���
	CAN_InitStructure.CAN_TXFP = DISABLE; //MCR-TXFP ����FIFO���ȼ� DISABLE-���ȼ�ȡ���ڱ��ı�ʾ��
#if 0
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	//CAN_Mode_Normal-����ģʽ;CAN_Mode_LoopBack-�ػ�ģʽ
#else
	CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;	//CAN_Mode_Normal-����ģʽ;CAN_Mode_LoopBack-�ػ�ģʽ
#endif
	CAN_InitStructure.CAN_SJW = CAN_SJW_2tq; 	//BTR-SJW ����ͬ����Ծʱ����2��ʱ�䵥Ԫ
	
#if 0//
	CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq; 	//BTR-TS1 ʱ���1ռ��6��ʱ�䵥Ԫ
	CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq; 	//BTR-TS1 ʱ���2ռ��3��ʱ�䵥Ԫ
	CAN_InitStructure.CAN_Prescaler = 4; 			//BTR-BRP �����ʷ�Ƶ�� ʱ�䳤��36/(1+6+3)/4=0.9Mbps
#else
	CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq; 	//BTR-TS1 ʱ���1ռ��6��ʱ�䵥Ԫ
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq; 	//BTR-TS1 ʱ���2ռ��3��ʱ�䵥Ԫ
	CAN_InitStructure.CAN_Prescaler = 12; 		//BTR-BRP �����ʷ�Ƶ�� ʱ�䳤��36/(1+3+2)/12=0.5Mbps
#endif
CAN_Init(CAN1, &CAN_InitStructure);   
}

/*
 * CAN ����������
 */
static void CAN_Filter_Config(void)
{
	CAN_FilterInitTypeDef CAN_FilterInitStructure;

	/*CAN ��������ʼ��*/
	CAN_FilterInitStructure.CAN_FilterNumber = 0;										//�������� 0
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;	//�����ڱ�ʶ������λģʽ
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//λ��Ϊ���� 32λ
	
	/*���ձ�ʶ�������ݽ��бȶԹ���*/
	
#if CAN_STD//��׼����֡
	CAN_FilterInitStructure.CAN_FilterIdHigh = (((u32)0x12<<21)&0xFFFF0000)>>16;									//Ҫ���� ID �ĸ�λ
	CAN_FilterInitStructure.CAN_FilterIdLow = (((u32)0x1314<<21)|CAN_ID_STD|CAN_RTR_DATA)&0xFFFF;	//Ҫ���� ID �ĵ�λ
#else//��չ����֡
	CAN_FilterInitStructure.CAN_FilterIdHigh=(((u32)0x1314<<3)&0xFFFF0000)>>16;									//Ҫ���� ID �ĸ�λ
	CAN_FilterInitStructure.CAN_FilterIdLow=(((u32)0x1314<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;	//Ҫ���� ID �ĵ�λ
#endif
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0xFFFF;	//�������� 16 λ
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0xFFFF;		//�������� 16 λ
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;	//�������������� FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;	//ʹ�ܹ�����
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	/*CAN ͨ���ж�ʹ��*/
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}

/**
 * CAN GPIO ����:PB8-�������� PB8-�������
 */
static void CAN_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* ���ù��ܺ�GPIOB�˿�ʱ��ʹ��*/ 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
	/* CAN1 ģ��ʱ��ʹ�� */ 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	/* CAN1 GPIO config */
	GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
	/* Configure CAN1 pin:Rx (PB.8)*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure CAN1 pin:Tx (PB.9) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);    
}

/**
 * CAN ����
 */
void CAN_Config(void)
{
	CAN_GPIO_Config();
	CAN_NVIC_Config();
	CAN_Mode_Config();
	CAN_Filter_Config();
}

/*
 * CAN ͨ�ű�����������
 */
void CAN_SetMsg(void)
{
#if CAN_STD
	TxMessage.StdId = 0x12;
	TxMessage.IDE = CAN_ID_STD;
#else
	TxMessage.ExtId = 0x1314;				//ʹ�õ���չID
	TxMessage.IDE = CAN_ID_EXT; 		//��չģʽ
#endif	
	TxMessage.RTR = CAN_RTR_DATA; 	//���͵�������
	TxMessage.DLC = 2; 							//���ݳ��� 2 �ֽ�
	TxMessage.Data[0] = 0xAB;
	TxMessage.Data[1] = 0xCD;
}

/*CAN���Ժ���*/
void CAN_Test(void)
{
	printf( "\r\n***** CAN ͨѶʵ��(�ػ�����): ******** \r\n");
	/*����ͨ��can���͵���Ϣ*/
	CAN_SetMsg();
	
	printf( "\r\n***** CAN ���ͱ�������: ********");
#if CAN_STD
	printf( "\r\n***** CAN ��׼ID��: 0x%x, ���ݶ�����: Data[0]=0x%x, Data[1]=0x%x\r\n", TxMessage.StdId, TxMessage.Data[0], TxMessage.Data[1]);
#else
	printf( "\r\n***** CAN ��չID��: 0x%x, ���ݶ�����: Data[0]=0x%x, Data[1]=0x%x\r\n", TxMessage.ExtId, TxMessage.Data[0], TxMessage.Data[1]);
#endif
	/* ������Ϣ ��ABCD�� */
	CAN_Transmit(CAN1, &TxMessage);
	
	while (0xff == can_flag)
		;
	
	printf( "\r\n***** CAN ���ձ�������: ********");
#if CAN_STD
	printf( "\r\n***** CAN ��׼ID��: 0x%x, ���ݶ�����: Data[0]=0x%x, Data[1]=0x%x \r\n", RxMessage.StdId, RxMessage.Data[0], RxMessage.Data[1]);
#else
	printf( "\r\n***** CAN ��չID��: 0x%x, ���ݶ�����: Data[0]=0x%x, Data[1]=0x%x \r\n", RxMessage.ExtId, RxMessage.Data[0], RxMessage.Data[1]);
#endif
}
/*********************************************END OF FILE**********************/
