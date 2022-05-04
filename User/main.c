/********************************************************************************
  * @file    HCSR04 + DS18B20 + DS1302 + NRF + MLX + DS1302 + HY2615 + CAN +
  *          FreeRTOS
  * @author  liu xu
  * @version V1.0
  * @date    2022-5-3
  * @node    ��ֲFreeRTOS�������޸���ϵͳʱ�ӣ��Զ���ľ�ȷ��ʱ����Delay_us()���޷�ʹ��
  ******************************************************************************/ 
#include "stm32f10x.h"
#include "bsp_led.h"
#include "bsp_usart1.h"
#include "bsp_SysTick.h"
//#include "bsp_hcsr04.h"
//#include "bsp_ds18b20.h"
//#include "bsp_spi_nrf.h"
//#include "bsp_mlx90614.h"
//#include "bsp_dht11.h"
//#include "bsp_ds1302.h"
//#include "bsp_hy2615.h"
#include "bsp_timer.h"
#include "bsp_key.h"
//#include "bsp_can.h"
#include "FreeRTOS.h"
#include "task.h"

/********************** ������ *************************/
/*
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ����������
 * ����������Ҫͨ����������������������������Լ�����ô�������ΪNULL��
 */
/*����������*/
static TaskHandle_t AppTaskCreate_Handle;
/*LED������*/
static TaskHandle_t LED_Task_Handle;

/********************** �ں˶����� *************************/
/*
 * �ź�������Ϣ���С��¼���־�顢�����ʱ���������ں˶���
 * ʹ��ʱ�����ȴ����������ɹ��󷵻�һ�������ʵ����һ��ָ�롣
 * �ں˶���˵���˾���һ��ȫ�ֵ����ݽṹ����ʵ�������ͨ�š�������¼�ͬ���ȸ��ֹ��ܡ�
 */

/********************** ȫ�ֱ������� *************************/
/*App�����ջ*/
static StackType_t AppTaskCreate_Stack[128];
/*LED�����ջ*/
static StackType_t LED_Task_Stack[128];
/*���������ջ*/
static StackType_t Idle_Task_Stack[configMINIMAL_STACK_SIZE];
/*��ʱ�������ջ*/
static StackType_t Timer_Task_Stack[configTIMER_TASK_STACK_DEPTH];

/*App������ƿ�*/
static StaticTask_t AppTaskCreate_TCB;
/*LED������ƿ�*/
static StaticTask_t LED_Task_TCB;
/*����������ƿ�*/
static StaticTask_t Idle_Task_TCB;
/*��ʱ��������ƿ�*/
static StaticTask_t Timer_Task_TCB;

/*************************** �������� ****************************/
static void AppTaskCreate(void); 				/*���ڴ�������*/
static void LED_Task(void *pvParameters);		/*LED_Task����ʵ��*/
static void BSP_Init(void); /*��ʼ�����������Դ*/
void Delay(__IO uint32_t nCount); /*�򵥵���ʱ����*/
/*
 * ʹ�þ�̬�����ڴ棬����2���������û�ʵ�֣�������task.c��������
 * ����configSUPPORT_STATIC_ALLOCATIONΪ1ʱ��Ч
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
												    StackType_t **ppxTimerTaskStackBuffer, 
												        uint32_t *pulTimerTaskStackSize); 
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
	                                             StackType_t **ppxIdleTaskStackBuffer, 
	                                                 uint32_t *pulIdleTaskStackSize);

/* ������
 * 1��Ӳ����ʼ����2������APPӦ������3������FreeTRTOS����ʼ���������
 */
int main(void)
{	
	BSP_Init(); //�����ʼ��
	
	printf("\n\nDS18B20 + HCSR04 + NRF + MLX90614 + DHT11 + DS1302 + CAN + RTOS\n\n");

	/*����APP����*/
	AppTaskCreate_Handle = xTaskCreateStatic((TaskFunction_t)AppTaskCreate,
											 (const char *)"AppTaskCreate", //��������
											 (uint32_t)128,					//�����ջ��С
											 (void *)NULL,					//���ݸ�����Ĳ���
											 (UBaseType_t)3,				//�������ȼ�
											 (StackType_t *)AppTaskCreate_Stack,
											 (StaticTask_t *)&AppTaskCreate_TCB);
	if (NULL != AppTaskCreate_Handle)
	{
		/*�����ɹ������������񣬿�������*/
		vTaskStartScheduler();
	}

	/*��������ִ�е�����*/
	while (1)
	{		
		;
	}
}

/*Ϊ�˷���������е����񴴽���������������*/
static void AppTaskCreate(void)
{
	/*�����ٽ���*/
	taskENTER_CRITICAL();

	/*����LED_Task����*/
	LED_Task_Handle = xTaskCreateStatic((TaskFunction_t)LED_Task, 	//������
										(const char *)"LED_Task",	//��������
										(uint32_t)128, 				//�����ջ��С
										(void *)NULL,				//���ݸ��������Ĳ���
										(UBaseType_t)4,				//�������ȼ�
										(StackType_t *)LED_Task_Stack,	//�����ջ
										(StaticTask_t *)&LED_Task_TCB);	//������ƿ�

	if (NULL != LED_Task_Handle)
	{
		printf("LED_Task ���񴴽��ɹ�!\n");
	}
	else
	{
		printf("LED_Task ���񴴽�ʧ��!\n");
	}

	/*ɾ��App����*/
	vTaskDelete(AppTaskCreate_Handle);

	/*�Ƴ��ٽ���*/
	taskEXIT_CRITICAL();
}

/*LED ������Ժ���*/
static void LED_Task(void *parameter)
{
	while (1)
	{
		LED1_ON;
		LED2_ON;
		vTaskDelay(500); //��ʱ500��tick

		LED1_OFF;
		LED2_OFF;
		vTaskDelay(500); //��ʱ500��tick
	}
}

/*�弶�����ʼ��*/
static void BSP_Init(void)
{
	/* 
	 * STM32 �ж����ȼ�����Ϊ 4���� 4bit ��������ʾ��ռ���ȼ�����ΧΪ��0~15 
	 * ���ȼ�����ֻ��Ҫ����һ�μ��ɣ��Ժ������������������Ҫ�õ��жϣ� 
	 * ��ͳһ��������ȼ����飬ǧ��Ҫ�ٷ��飬�мɡ� 
	 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	LED_GPIO_Config();		        // ��ʼ�� LED�˿� 
	KEY_GPIO_Config();		        // ��ʼ�� KEY�˿� 
	USART1_Config();				// ��ʼ�� ����
	SysTick_Init();	               	// ����ϵͳʱ��Ϊ1us�ж�һ�� 
	TIM6_Init();	               	// ��ʼ�� TIM6 
	//HCSR04_Init();				// ��ʼ�� HCSR04
	//DS18B20_Init();				// ��ʼ�� DS18B20
	//SPI_NRF_Init();	            // ��ʼ�� NRF 
	//SPI_NRF2_Init();	            // ��ʼ�� NRF2 
	//SMBus_Init();					// ��ʼ�� SMBus����(mlx90614)
	//DHT11_GPIO_Config();		    // ��ʼ�� DHT11�˿� 
	//DS1302_Init();	
	//HY2615_Init(); 				// ��ʼ�� HY2615 IIC
	//CAN_Config(); 				// ��ʼ�� CAN   ���Ժ����ӿڣ�CAN_Test()
}

/* ��ȡ��������Ķ�ջ�Ϳ��ƿ��ڴ� */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
												 StackType_t **ppxIdleTaskStackBuffer, 
												     uint32_t *pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer = &Idle_Task_TCB; 			/*������ƿ��ڴ�*/
	*ppxIdleTaskStackBuffer = Idle_Task_Stack; 			/*�����ջ�ڴ�*/
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE; 	/*�����ջ��С*/
}

/* ��ȡ��ʱ������Ķ�ջ�Ϳ��ƿ��ڴ� */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
											        StackType_t **ppxTimerTaskStackBuffer, 
												        uint32_t *pulTimerTaskStackSize)
{
	*ppxTimerTaskTCBBuffer = &Timer_Task_TCB;			 	/*������ƿ��ڴ�*/
 	*ppxTimerTaskStackBuffer = Timer_Task_Stack; 		 	/*�����ջ�ڴ�*/
 	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;	/*�����ջ��С*/
}

/*�򵥵���ʱ����*/
void Delay(__IO uint32_t nCount)
{
	for(; nCount != 0; nCount--);
}

/********************************  ���ú���  *********************************/
/******	���Ժ�����	*******/
//CAN_Test();
//printf("\nTEST\n");	
//	uint8_t aa[8] = {0};
//	uint8_t i = 0;
//	DS18B20_Write_Byte(0x33);	 // �����к�(0x28472310040000fe)
//	for (i=0;i<8;i++)
//		aa[i] = DS18B20_Read_Byte();
//	printf("\nDS18B20 Number:0x");
//	for (i=0;i<8;i++)
//		printf("%.2x", aa[i]);
//  NRF_Test();								 		 // ���NRF��״̬���Ƿ����ӳɹ�
//		printf("HCSR04-Length = %.2f\n", HCSR04_GetLength());	      // ��ӡ����
//		printf("\nDS18B20-Temperture = %.2f\n", DS18B20_Get_Temp());// ��ӡ�¶�

/******	�������mlx90614	*******/
// SMBus_Init();									 // SMBus���� ��ʼ��(mlx90614)
//	  printf("\nMLX90614-Temperture = %.2f\n", SMBus_ReadTemp());// ��ӡ�¶�(90614)

/******	ʱ��ds1302	*******/
//DS1302_Time_Show();
		
//		Show_DHT11(); // ��ʾ DHT11 ��ȡ���¡�ʪ������
//	  NRF_Mode1(); // Mode1:  NRF - TX_Mode + NRF2 - RX_Mode
//	  NRF_Mode2(); // Mode2:  NRF2 - TX_Mode + NRF - RX_Mode
//Delay_us(1000 * 1000);       //1s ��ȷ��ʱ
/*********************************************END OF FILE**********************/
