/********************************************************************************
  * @file    HCSR04 + DS18B20 + DS1302 + NRF + MLX + HY2615 + DHT11 + CAN +
  *          FreeRTOS
  * @author  liu xu
  * @version V1.0
  * @date    2022-5-3
  * @node    移植FreeRTOS后，由于修改了系统时钟，自定义的精确延时函数Delay_us()暂无法使用
  ******************************************************************************/ 
#include "stm32f10x.h"
/*Free RTOS头文件*/
#include "FreeRTOS.h"
#include "task.h"
/*bsp头文件*/
#include "bsp_led.h"
#include "bsp_usart1.h"
#include "bsp_SysTick.h"
#include "bsp_dwt.h"
//#include "bsp_hcsr04.h"
//#include "bsp_rtos_ds18b20.h"
//#include "bsp_spi_nrf.h"
//#include "bsp_mlx90614.h"
//#include "bsp_dht11.h"
//#include "bsp_ds1302.h"
//#include "bsp_hy2615.h"
#include "bsp_timer.h"
#include "bsp_key.h"
#include "bsp_can.h"
#include "user_data_conf.h"

#if 1//动态内存
/********************** 任务句柄 *************************/
/*
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄。
 * 操作任务都需要通过这个句柄。如果是自身的任务操作自己，那么句柄可以为NULL。
 */
/*创建任务句柄*/
static TaskHandle_t AppTaskCreate_Handle = NULL;
/*LED 任务句柄*/
static TaskHandle_t Task_Handle_LED1 = NULL;
static TaskHandle_t Task_Handle_LED2 = NULL;
/*Key 任务句柄*/
static TaskHandle_t Task_Handle_Key2 = NULL;
/*Usart 任务句柄*/
static TaskHandle_t Task_Handle_Usart = NULL;
/*ds18b20 任务句柄*/
static TaskHandle_t Task_Handle_DS18B20 = NULL;
/*CAN 任务句柄*/
static TaskHandle_t Task_Handle_CAN = NULL;
/*Test 任务句柄*/
static TaskHandle_t Task_Handle_Test = NULL;

/********************** 内核对象句柄 *************************/
/*
 * 信号量、消息队列、事件标志组、软件定时器都属于内核对象。
 * 使用时必须先创建，创建成功后返回一个句柄，实际是一个指针。
 * 内核对象说白了就是一种全局的数据结构，来实现任务间通信、任务间事件同步等各种功能。
 */

/*************************** 全局变量声明 *************************/

/*************************** 函数声明 *****************************/
static void AppTaskCreate(void);				/*用于创建任务*/
static void Task_LED1(void *pvParameters);	/*LED_Task任务实现*/
static void Task_LED2(void *pvParameters);	/*LED_Task任务实现*/
static void Task_Key2(void *pvParameters);	/*Key_Task任务实现*/
static void Task_Usart(void *pvParameters);	/*Usart_Task任务实现*/
static void Task_Test(void *pvParameters);
static void Task_DS18B20(void *pvParameters);
static void Task_CAN(void *pvParameters);
static void BSP_Init(void);	/*初始化板载相关资源*/

/* 主函数
 * 1：硬件初始化；2：创建APP应用任务；3：启动FreeRTOS，开始多任务调度
 */
int main(void)
{	
	BaseType_t xReturn = pdPASS;

	BSP_Init(); //外设初始化
	
	printf("\n\nDS18B20 + HCSR04 + NRF + MLX90614 + DHT11 + DS1302 + CAN + RTOS\n\n");

	/*动态创建APP任务*/
	xReturn = xTaskCreate((TaskFunction_t)AppTaskCreate,	//任务入口函数
						  (const char *)"AppTaskCreate",	//任务名称
						  (uint16_t)512,					//任务堆栈大小
						  (void *)NULL,						//传递给任务的参数
						  (UBaseType_t)TASK_PRI_APP,		//任务优先级
						  (TaskHandle_t *)&AppTaskCreate_Handle);	//任务控制块指针
	if (pdPASS == xReturn)
	{
		/*创建成功，则启动任务，开启调度。不会返回，从此任务都由FreeRTOS管理*/
		vTaskStartScheduler();
	}
	else
	{
		printf("AppTaskCreate error\n");
		return -1;
	}

	/*正常不会执行到这里*/
	while (1);
}

/*为了方便管理，所有的任务创建函数都放在这里*/
static void AppTaskCreate(void)
{
	BaseType_t xReturn = pdPASS;
	
	/*进入临界区*/
	taskENTER_CRITICAL();

	/*创建LED1_Task任务*/
	xReturn = xTaskCreate((TaskFunction_t)Task_LED1,	//任务函数
						  (const char *)"Task_LED1",	//任务名称
						  (uint16_t)512, 				//任务堆栈大小
						  (void *)NULL,					//传递给任务函数的参数
						  (UBaseType_t)TASK_PRI_LED1,	//任务优先级
						  (TaskHandle_t *)&Task_Handle_LED1);	//任务控制块指针

	if (pdPASS == xReturn)
	{
		printf("LED1_Task 任务创建成功!\n");
	}
	else
	{
		printf("LED1_Task 任务创建失败!\n");
	}
	
	/*创建LED2_Task任务*/
	xReturn = xTaskCreate((TaskFunction_t)Task_LED2,	//任务函数
						  (const char *)"Task_LED2",	//任务名称
						  (uint16_t)512, 				//任务堆栈大小
						  (void *)NULL,					//传递给任务函数的参数
						  (UBaseType_t)TASK_PRI_LED2,	//任务优先级
						  (TaskHandle_t *)&Task_Handle_LED2);	//任务控制块指针

	if (pdPASS == xReturn)
	{
		printf("LED2_Task 任务创建成功!\n");
	}
	else
	{
		printf("LED2_Task 任务创建失败!\n");
	}
	
#if 0
	/*创建Key2_Task任务*/
	xReturn = xTaskCreate((TaskFunction_t)Task_Key2,	//任务函数
						  (const char *)"Task_Key2",	//任务名称
						  (uint16_t)512, 			//任务堆栈大小
						  (void *)NULL,				//传递给任务函数的参数
						  (UBaseType_t)4,			//任务优先级
						  (TaskHandle_t *)&Task_Handle_Key2);	//任务控制块指针

	if (pdPASS == xReturn)
	{
		printf("Key2_Task 任务创建成功!\n");
	}
	else
	{
		printf("Key2_Task 任务创建失败!\n");
	}
#endif
#if 0
	/*创建Usart_Task任务*/
	xReturn = xTaskCreate((TaskFunction_t)Task_Usart,	//任务函数
						  (const char *)"Task_Usart",	//任务名称
						  (uint16_t)512, 				//任务堆栈大小
						  (void *)NULL,					//传递给任务函数的参数
						  (UBaseType_t)TASK_PRI_USART,	//任务优先级
						  (TaskHandle_t *)&Task_Handle_Usart);	//任务控制块指针

	if (pdPASS == xReturn)
	{
		printf("Usart_Task 任务创建成功!\n");
	}
	else
	{
		printf("Usart_Task 任务创建失败!\n");
	}	
#endif
#if 0
	/*创建Task_Test任务*/
	xReturn = xTaskCreate((TaskFunction_t)Task_Test,	//任务函数
						  (const char *)"Task_Test",	//任务名称
						  (uint16_t)512, 			//任务堆栈大小
						  (void *)NULL,				//传递给任务函数的参数
						  (UBaseType_t)7,			//任务优先级
						  (TaskHandle_t *)&Task_Handle_Test);	//任务控制块指针

	if (pdPASS == xReturn)
	{
		printf("Task_Test 任务创建成功!\n");
	}
	else
	{
		printf("Task_Test 任务创建失败!\n");
	}
#endif

#if 0//DS18B20
	/*创建Task_DS18B20任务*/
	xReturn = xTaskCreate((TaskFunction_t)Task_DS18B20,	//任务函数
						  (const char *)"Task_DS18B20",	//任务名称
						  (uint16_t)512,				//任务堆栈大小
						  (void *)NULL, 				//传递给任务函数的参数
						  (UBaseType_t)TASK_PRI_DS18B20,//任务优先级
						  (TaskHandle_t *)&Task_Handle_DS18B20);	//任务控制块指针

	if (pdPASS == xReturn)
	{
		printf("Task_DS18B20 任务创建成功!\n");
	}
	else
	{
		printf("Task_DS18B20 任务创建失败!\n");
	}
#endif
	
#if 1//CAN
	/*创建Task_CAN任务*/
	xReturn = xTaskCreate((TaskFunction_t)Task_CAN,	//任务函数
						  (const char *)"Task_CAN",	//任务名称
						  (uint16_t)512,				//任务堆栈大小
						  (void *)NULL, 				//传递给任务函数的参数
						  (UBaseType_t)TASK_PRI_CAN,//任务优先级
						  (TaskHandle_t *)&Task_Handle_CAN);	//任务控制块指针

	if (pdPASS == xReturn)
	{
		printf("Task_CAN 任务创建成功!\n");
	}
	else
	{
		printf("Task_CAN 任务创建失败!\n");
	}
#endif

	/*删除App任务*/
	vTaskDelete(AppTaskCreate_Handle);

	/*退出临界区*/
	taskEXIT_CRITICAL();
}

/*LED1 任务测试函数*/
static void Task_LED1(void *parameter)
{
	while (1)
	{
		LED1_TOGGLE;
		vTaskDelay(2*1000); //延时N个tick
		//dwt_delay_ms(1*1000);
	}
}

/*LED2 任务测试函数*/
static void Task_LED2(void *parameter)
{
	static portTickType PreviousWakeTime; //任务最后一次解除阻塞的时间，调用后系统自动更新
	const portTickType TimeIncrement = pdMS_TO_TICKS(1000); // 设置延时时间，将时间转为节拍数

	PreviousWakeTime = xTaskGetTickCount(); // 获取当前系统时间
		 
	LED2_ON;
	while (1)
	{
		#if 1
		/*绝对延时*/
		vTaskDelayUntil(&PreviousWakeTime, TimeIncrement);
		#else
		/*相对延时*/
		vTaskDelay(500);
		#endif
		LED2_TOGGLE;

		#if 0//vTaskDelete 函数测试
		/*删除任务本身*/
		vTaskDelete(NULL);
		#elif 0
		/*删除其他任务*/
		if (NULL != Task_Handle_LED1)
		{
			vTaskDelete(Task_Handle_LED1);
		}
		#else
		;
		#endif
	}
}

/*Key 任务测试函数*/
static void Task_Key2(void *parameter)
{
	while (1)
	{
		/*扫描KEY2*/
		if (KEY_ON == Key_Scan(KEY2_GPIO_PORT, KEY2_GPIO_PIN))
		{
			printf("suspend led task\n");
			vTaskSuspend(Task_Handle_LED1); 	//挂起单个任务
			//vTaskSuspendAll();					//挂起所有任务
		}
		/*扫描KEY3*/
		if (KEY_ON == Key_Scan(KEY3_GPIO_PORT, KEY3_GPIO_PIN))
		{
			printf("resume led task\n");
			vTaskResume(Task_Handle_LED1); 	//恢复单个任务
			//xTaskResumeAll();					//恢复所有任务
		}

		vTaskDelay(20); //延时*个tick
	}
}

/*Usart 任务测试函数*/
static void Task_Usart(void *parameter)
{
	while (1)
	{
		printf("Usart_Task test!\n");
		
		#if 1
		vTaskDelay(5*1000); //延时N个tick
		#else
		dwt_delay_ms(5*1000);
		//dwt_delay_us(5*1000*1000);
		#endif
	}
}

#if 0
/*ds18b20 任务函数*/
static void Task_DS18B20(void *parameter)
{
	while (1)
	{
		printf("DS18B20-Temperture = %.2f\n", DS18B20_rtos_Get_Temp());// 打印温度
		
		vTaskDelay(5*1000); //延时N个tick
	}
}
#endif

/*CAN 任务函数*/
static void Task_CAN(void *parameter)
{
	while (1)
	{
		printf("CAN TEST\n");
		CAN_Test();
		
		vTaskDelay(10*1000); //延时N个tick
	}
}

/*Test 任务测试函数*/
static void Task_Test(void *parameter)
{
	while (1)
	{
		printf("Task_Test test!\n");
		
		#if 1
		vTaskDelay(10*1000); //延时N个tick
		#elif 0
		vTaskDelay(10*1000); //延时N个tick
		vTaskSuspendAll();
		vTaskDelay(10*1000); //延时N个tick
		vTaskSuspendAll();
		vTaskDelay(10*1000); //延时N个tick
		vTaskSuspendAll();

		vTaskDelay(10*1000); //延时N个tick
		xTaskResumeAll();
		vTaskDelay(10*1000); //延时N个tick
		xTaskResumeAll();
		vTaskDelay(10*1000); //延时N个tick
		xTaskResumeAll();
		#else
		vTaskDelay(10*1000); //延时N个tick
		vTaskSuspend(Task_Handle_LED2); 	//挂起单个任务

		vTaskDelay(10*1000); //延时N个tick
		vTaskResume(Task_Handle_LED2); 	//挂起单个任务
		#endif
	}
}

/*板级外设初始化*/
static void BSP_Init(void)
{
	dwt_delay_init();				// 初始化 dwt
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
	//DS18B20_rtos_Init();			// 初始化 DS18B20
	//SPI_NRF_Init();	            // 初始化 NRF 
	//SPI_NRF2_Init();	            // 初始化 NRF2 
	//SMBus_Init();					// 初始化 SMBus总线(mlx90614)
	//DHT11_GPIO_Config();		    // 初始化 DHT11端口 
	//DS1302_Init();	
	//HY2615_Init(); 				// 初始化 HY2615 IIC
	CAN_Config(); 				// 初始化 CAN 测试函数接口：CAN_Test()
}
#endif

#if 0//静态内存
/********************** 任务句柄 *************************/
/*
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄。
 * 操作任务都需要通过这个句柄。如果是自身的任务操作自己，那么句柄可以为NULL。
 */
/*创建任务句柄*/
//static TaskHandle_t AppTaskCreate_Handle;
static TaskHandle_t AppTaskCreate_Handle = NULL;
/*LED任务句柄*/
//static TaskHandle_t LED_Task_Handle;
static TaskHandle_t LED_Task_Handle = NULL;

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

	/*退出临界区*/
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
#endif

//void Delay(__IO uint32_t nCount); /*简单的延时函数*/
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
