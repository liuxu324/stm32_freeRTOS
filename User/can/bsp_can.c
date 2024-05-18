/*****************************************************
* CAN 差分信号的表示
* 1:隐性电平 H2.5v - L2.5v = 0v
* 0:显性电平 H3.5v - L1.5v = 2v
*****************************************************/ 
#include "bsp_can.h"
#include <stdio.h>

volatile unsigned char can_flag;

/*变量*/
CanTxMsg TxMessage; // 发送缓冲区
CanRxMsg RxMessage; // 接收缓冲区

/* 配置CAN1接收中断 */
static void CAN_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	/* Enable the CANy Interrupt */ 
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
/**
 * CAN 模式配置
 */
static void CAN_Mode_Config(void)
{
	CAN_InitTypeDef CAN_InitStructure;
	
	/* CAN1 param config */
	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);
	
	/* CAN1 unit config */
	CAN_InitStructure.CAN_TTCM=DISABLE;	//MCR-TTCM 关闭时间触发通信模式使能
	CAN_InitStructure.CAN_ABOM=ENABLE; 	//MCR-ABOM 自动离线管理
	CAN_InitStructure.CAN_AWUM=ENABLE; 	//MCR-AWUM 使用自动唤醒
	CAN_InitStructure.CAN_NART=DISABLE; //MCR-NART 禁止自动重传
	CAN_InitStructure.CAN_RFLM=DISABLE; //MCR-RFLM 接收FIFO锁定模式 DISABLE-溢出时新报文会覆盖原有报文
	CAN_InitStructure.CAN_TXFP=DISABLE; //MCR-TXFP 发送FIFO优先级 DISABLE-优先级取决于报文标示符
	CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;	//CAN_Mode_Normal-正常模式;CAN_Mode_LoopBack-回环模式
	CAN_InitStructure.CAN_SJW=CAN_SJW_2tq; 	//BTR-SJW 重新同步跳跃时间宽度2个时间单元
	CAN_InitStructure.CAN_BS1=CAN_BS1_6tq; 	//BTR-TS1 时间段1占用6个时间单元
	CAN_InitStructure.CAN_BS2=CAN_BS2_3tq; 	//BTR-TS1 时间段2占用3个时间单元
	CAN_InitStructure.CAN_Prescaler =4; 	//BTR-BRP 波特率分频器 时间长：36/(1+6+3)/4=0.9Mbps
	CAN_Init(CAN1, &CAN_InitStructure);   
}

/*
 * CAN 过滤器配置
 */
static void CAN_Filter_Config(void)
{
	CAN_FilterInitTypeDef CAN_FilterInitStructure;

	/*CAN 过滤器初始化*/
	CAN_FilterInitStructure.CAN_FilterNumber=0;	//过滤器组 0
	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;	//工作在标识符屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;	//位宽为单个 32位
	
	/*按照标识符的内容进行比对过滤*/
	CAN_FilterInitStructure.CAN_FilterIdHigh=(((u32)0x1314<<3)&0xFFFF0000)>>16;	//要过滤 ID 的高位
	CAN_FilterInitStructure.CAN_FilterIdLow=(((u32)0x1314<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;	//要过滤 ID 的底位
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh= 0xFFFF;	//过滤器高 16 位
	CAN_FilterInitStructure.CAN_FilterMaskIdLow= 0xFFFF;	//过滤器低 16 位
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;	//过滤器被关联到 FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;	//使能过滤器
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	/*CAN 通信中断使能*/
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}

/**
 * CAN GPIO 配置:PB8-上拉输入 PB8-推挽输出
 */
static void CAN_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* config CAN1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	/* CAN1 GPIO config */
	GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
	/* Configure CAN1 pin:Rx (PB.8)*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure CAN1 pin:Tx (PB.9) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);    
}

/**
 * CAN 配置
 */
void CAN_Config(void)
{
	CAN_GPIO_Config();
	CAN_NVIC_Config();
	CAN_Mode_Config();
	CAN_Filter_Config();
}

/*
 * CAN 通信报文内容设置
 */
void CAN_SetMsg(void)
{
	//TxMessage.StdId=0x00;
	TxMessage.ExtId=0x1314;			//使用的扩展ID
	TxMessage.IDE=CAN_ID_EXT; 	//扩展模式
	TxMessage.RTR=CAN_RTR_DATA; //发送的是数据
	TxMessage.DLC=2; 						//数据长度 2 字节
	TxMessage.Data[0]=0xAB;
	TxMessage.Data[1]=0xCD;
}

/*CAN测试函数*/
void CAN_Test(void)
{
	printf( "\r\n***** CAN 通讯实验(回环测试): ******** \r\n");
	/*设置通过can发送的消息*/
	CAN_SetMsg();
	
	printf( "\r\n***** CAN 发送报文内容: ********");
	printf( "\r\n***** CAN 扩展ID号: 0x%x, 数据段内容:Data[0]=0x%x, Data[1]=0x%x\r\n", TxMessage.ExtId, TxMessage.Data[0], TxMessage.Data[1]);
	/* 发送消息 “ABCD” */
	CAN_Transmit(CAN1, &TxMessage);
	
	while (0xff == can_flag)
		;
	
	printf( "\r\n***** CAN 接收报文内容: ********");
	printf( "\r\n***** CAN 扩展ID号: 0x%x, 数据段内容: Data[0]=0x%x,Data[1]=0x%x \r\n", RxMessage.ExtId, RxMessage.Data[0], RxMessage.Data[1]);
}
/*********************************************END OF FILE**********************/
