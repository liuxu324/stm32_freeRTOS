/********************************************************************************
  * @file    HCSR04 + DS18B20 + DS1302 + NRF + MLX + DS1302 + HY2615 + CAN +
  *          FreeRTOS
  * @author  liu xu
  * @version V1.0
  * @date    2022-5-3
  * @node    移植FreeRTOS后，由于修改了系统时钟，自定义的精确延时函数Delay_us()暂无法使用
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

/********************** 任务句柄 *************************/
/*
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄。
 * 操作任务都需要通过这个句柄。如果是自身的任务操作自己，那么句柄可以为NULL。
 */
/*创建任务句柄*/
static TaskHandle_t AppTaskCreate_Handle;
/*LED任务句柄*/
static TaskHandle_t LED_Task_Handle;

/********************** 内核对象句柄 *************************/
/*
 * 信号量、消息队列、事件标志组、软件定时器都属于内核对象。
 * 使用时必须先创建，创建成功后返回一个句柄，实际是一个指针。
 * 内核对象说白了就是一种全局的数据结构，来实现任务间通信、任务间事件同步等各种功能。
 */

/********************** 全局变量声明 *************************/
/*App任务堆栈*/
static StackType_t AppTaskCreate_Stack[128];
/*LED任务堆栈*/
static StackType_t LED_Task_Stack[128];
/*空闲任务堆栈*/
static StackType_t Idle_Task_Stack[configMINIMAL_STACK_SIZE];
/*定时器任务堆栈*/
static StackType_t Timer_Task_Stack[configTIMER_TASK_STACK_DEPTH];

/*App任务控制块*/
static StaticTask_t AppTaskCreate_TCB;
/*LED任务控制块*/
static StaticTask_t LED_Task_TCB;
/*空闲任务控制块*/
static StaticTask_t Idle_Task_TCB;
/*定时器任务控制块*/
static StaticTask_t Timer_Task_TCB;

/*************************** 函数声明 ****************************/
static void AppTaskCreate(void); 				/*用于创建任务*/
static void LED_Task(void *pvParameters);		/*LED_Task任务实现*/
static void BSP_Init(void); /*初始化板载相关资源*/
void Delay(__IO uint32_t nCount); /*简单的延时函数*/
/*
 * 使用静态分配内存，如下2个函数由用户实现，函数在task.c中由引用
 * 当宏configSUPPORT_STATIC_ALLOCATION为1时有效
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
												    StackType_t **ppxTimerTaskStackBuffer, 
												        uint32_t *pulTimerTaskStackSize); 
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
	                                             StackType_t **ppxIdleTaskStackBuffer, 
	                                                 uint32_t *pulIdleTaskStackSize);

/* 主函数
 * 1：硬件初始化；2：创建APP应用任务；3：启动FreeTRTOS，开始多任务调度
 */
int main(void)
{	
	BSP_Init(); //外设初始化
	
	printf("\n\nDS18B20 + HCSR04 + NRF + MLX90614 + DHT11 + DS1302 + CAN + RTOS\n\n");

	/*创建APP任务*/
	AppTaskCreate_Handle = xTaskCreateStatic((TaskFunction_t)AppTaskCreate,
											 (const char *)"AppTaskCreate", //任务名称
											 (uint32_t)128,					//任务堆栈大小
											 (void *)NULL,					//传递给任务的参数
											 (UBaseType_t)3,				//任务优先级
											 (StackType_t *)AppTaskCreate_Stack,
											 (StaticTask_t *)&AppTaskCreate_TCB);
	if (NULL != AppTaskCreate_Handle)
	{
		/*创建成功，则启动任务，开启调度*/
		vTaskStartScheduler();
	}

	/*正常不会执行到这里*/
	while (1)
	{		
		;
	}
}

/*为了方便管理，所有的任务创建函数都放在这里*/
static void AppTaskCreate(void)
{
	/*进入临界区*/
	taskENTER_CRITICAL();

	/*创建LED_Task任务*/
	LED_Task_Handle = xTaskCreateStatic((TaskFunction_t)LED_Task, 	//任务函数
										(const char *)"LED_Task",	//任务名称
										(uint32_t)128, 				//任务堆栈大小
										(void *)NULL,				//传递给任务函数的参数
										(UBaseType_t)4,				//任务优先级
										(StackType_t *)LED_Task_Stack,	//任务堆栈
										(StaticTask_t *)&LED_Task_TCB);	//任务控制块

	if (NULL != LED_Task_Handle)
	{
		printf("LED_Task 任务创建成功!\n");
	}
	else
	{
		printf("LED_Task 任务创建失败!\n");
	}

	/*删除App任务*/
	vTaskDelete(AppTaskCreate_Handle);

	/*推出临界区*/
	taskEXIT_CRITICAL();
}

/*LED 任务测试函数*/
static void LED_Task(void *parameter)
{
	while (1)
	{
		LED1_ON;
		LED2_ON;
		vTaskDelay(500); //延时500个tick

		LED1_OFF;
		LED2_OFF;
		vTaskDelay(500); //延时500个tick
	}
}

/*板级外设初始化*/
static void BSP_Init(void)
{
	/* 
	 * STM32 中断优先级分组为 4，即 4bit 都用来表示抢占优先级，范围为：0~15 
	 * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断， 
	 * 都统一用这个优先级分组，千万不要再分组，切忌。 
	 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	LED_GPIO_Config();		        // 初始化 LED端口 
	KEY_GPIO_Config();		        // 初始化 KEY端口 
	USART1_Config();				// 初始化 串口
	SysTick_Init();	               	// 配置系统时钟为1us中断一次 
	TIM6_Init();	               	// 初始化 TIM6 
	//HCSR04_Init();				// 初始化 HCSR04
	//DS18B20_Init();				// 初始化 DS18B20
	//SPI_NRF_Init();	            // 初始化 NRF 
	//SPI_NRF2_Init();	            // 初始化 NRF2 
	//SMBus_Init();					// 初始化 SMBus总线(mlx90614)
	//DHT11_GPIO_Config();		    // 初始化 DHT11端口 
	//DS1302_Init();	
	//HY2615_Init(); 				// 初始化 HY2615 IIC
	//CAN_Config(); 				// 初始化 CAN   测试函数接口：CAN_Test()
}

/* 获取空闲任务的堆栈和控制块内存 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
												 StackType_t **ppxIdleTaskStackBuffer, 
												     uint32_t *pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer = &Idle_Task_TCB; 			/*任务控制块内存*/
	*ppxIdleTaskStackBuffer = Idle_Task_Stack; 			/*任务堆栈内存*/
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE; 	/*任务堆栈大小*/
}

/* 获取定时器任务的堆栈和控制块内存 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
											        StackType_t **ppxTimerTaskStackBuffer, 
												        uint32_t *pulTimerTaskStackSize)
{
	*ppxTimerTaskTCBBuffer = &Timer_Task_TCB;			 	/*任务控制块内存*/
 	*ppxTimerTaskStackBuffer = Timer_Task_Stack; 		 	/*任务堆栈内存*/
 	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;	/*任务堆栈大小*/
}

/*简单的延时函数*/
void Delay(__IO uint32_t nCount)
{
	for(; nCount != 0; nCount--);
}

/********************************  备用函数  *********************************/
/******	测试函数块	*******/
//CAN_Test();
//printf("\nTEST\n");	
//	uint8_t aa[8] = {0};
//	uint8_t i = 0;
//	DS18B20_Write_Byte(0x33);	 // 读序列号(0x28472310040000fe)
//	for (i=0;i<8;i++)
//		aa[i] = DS18B20_Read_Byte();
//	printf("\nDS18B20 Number:0x");
//	for (i=0;i<8;i++)
//		printf("%.2x", aa[i]);
//  NRF_Test();								 		 // 检测NRF的状态，是否连接成功
//		printf("HCSR04-Length = %.2f\n", HCSR04_GetLength());	      // 打印距离
//		printf("\nDS18B20-Temperture = %.2f\n", DS18B20_Get_Temp());// 打印温度

/******	红外测温mlx90614	*******/
// SMBus_Init();									 // SMBus总线 初始化(mlx90614)
//	  printf("\nMLX90614-Temperture = %.2f\n", SMBus_ReadTemp());// 打印温度(90614)

/******	时钟ds1302	*******/
//DS1302_Time_Show();
		
//		Show_DHT11(); // 显示 DHT11 获取的温、湿度数据
//	  NRF_Mode1(); // Mode1:  NRF - TX_Mode + NRF2 - RX_Mode
//	  NRF_Mode2(); // Mode2:  NRF2 - TX_Mode + NRF - RX_Mode
//Delay_us(1000 * 1000);       //1s 精确延时
/*********************************************END OF FILE**********************/
