#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f10x.h"
#include "bsp_usart1.h"

/*��Բ�ͬ�ı��������ò�ͬ��stdint.h�ļ�*/
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
#include <stdint.h>
extern uint32_t SystemCoreClock;
#endif

/*����*/
#define vAssertCalled(char,int) printf("Error:%s,%d\r\n",char,int)
#define configASSERT(x) if((x)==0) 	vAssertCalled(__FILE__,__LINE__)

/************************************************************************ 
 * FreeRTOS ������������ѡ�� 
 *********************************************************************/ 
/* �� 1��RTOS ʹ����ռʽ���������� 0��RTOS ʹ��Э��ʽ��������ʱ��Ƭ�� 
 * 
 * ע���ڶ������������ϣ�����ϵͳ���Է�Ϊ��ռʽ��Э��ʽ���֡� 
 * Э��ʽ����ϵͳ�����������ͷ� CPU ���л�����һ������ 
 * �����л���ʱ����ȫȡ�����������е����� 
 */
#define configUSE_PREEMPTION		(1)

/* �� 1��ʹ��ʱ��Ƭ���ȣ�Ĭ��ʹ�ܣ� */
#define configUSE_TIME_SLICING		(1)

/* ĳЩ���� FreeRTOS ��Ӳ�������ַ���ѡ����һ��Ҫִ�е����� 
 * ͨ�÷������ض���Ӳ���ķ��������¼�ơ����ⷽ�������� 
 * 
 * ͨ�÷����� 
 * 1.configUSE_PORT_OPTIMISED_TASK_SELECTION Ϊ 0 ����Ӳ����֧���������ⷽ���� 
 * 2.������������ FreeRTOS ֧�ֵ�Ӳ�� 
 * 3.��ȫ�� C ʵ�֣�Ч���Ե������ⷽ���� 
 * 4.��ǿ��Ҫ���������������ȼ���Ŀ 
 * 
 * ���ⷽ���� 
 * 1.���뽫 configUSE_PORT_OPTIMISED_TASK_SELECTION ����Ϊ 1�� 
 * 2.����һ�������ض��ܹ��Ļ��ָ�һ�������Ƽ���ǰ����[CLZ]ָ��� 
 * 3.��ͨ�÷�������Ч 
 * 4.һ��ǿ���޶����������ȼ���ĿΪ 32 
 * 
 * һ����Ӳ������ǰ����ָ������ʹ�õģ�MCU û����ЩӲ��ָ��Ļ��˺�Ӧ������Ϊ 0�� 
 */ 
#define configUSE_PORT_OPTIMISED_TASK_SELECTION	(1)

/* �� 1��ʹ�ܵ͹��� tickless ģʽ���� 0������ϵͳ���ģ�tick���ж�һֱ���� */ 
#define configUSE_TICKLESS_IDLE 			(1)

/* 
 * д��ʵ�ʵ� CPU �ں�ʱ��Ƶ�ʣ�Ҳ���� CPU ָ��ִ��Ƶ�ʣ�ͨ����Ϊ Fclk 
 * Fclk Ϊ���� CPU �ں˵�ʱ���źţ�������˵�� cpu ��ƵΪ XX MHz�� 
 * ����ָ�����ʱ���źţ���Ӧ�ģ�1/Fclk ��Ϊ cpu ʱ�����ڣ� 
 */ 
//#define configCPU_CLOCK_HZ				((unsigned long)72000000)	
#define configCPU_CLOCK_HZ 					(SystemCoreClock)
	
/* RTOS ϵͳ�����жϵ�Ƶ�ʡ���һ���жϵĴ�����ÿ���ж� RTOS �������������� */ 
#define configTICK_RATE_HZ					((TickType_t)1000)

/* ��ʹ�õ�������ȼ� */
#define configMAX_PRIORITIES				(32)

/* ��������ʹ�õĶ�ջ��С (��) */
#define configMINIMAL_STACK_SIZE			(( unsigned short)128)

/* ���������ַ������� */
#define configMAX_TASK_NAME_LEN				(16)

/* ϵͳ���ļ����������������ͣ�1 ��ʾΪ 16 λ�޷������Σ�0 ��ʾΪ 32 λ�޷������� */
#define configUSE_16_BIT_TICKS				(0)

/* ����������� CPU ʹ��Ȩ������ͬ���ȼ����û����� */
#define configIDLE_SHOULD_YIELD				(1)

/* ���ö��� */
#define configUSE_QUEUE_SETS				(1)

/* ��������֪ͨ���ܣ�Ĭ�Ͽ��� */
#define configUSE_TASK_NOTIFICATIONS		(1)

/* ʹ�û����ź��� */
#define configUSE_MUTEXES 					(1)
 
/* ʹ�õݹ黥���ź��� */
#define configUSE_RECURSIVE_MUTEXES 		(1) 
 
/* Ϊ 1 ʱʹ�ü����ź��� */
#define configUSE_COUNTING_SEMAPHORES 		(1) 

/* ���ÿ���ע����ź�������Ϣ���и��� */ 
#define configQUEUE_REGISTRY_SIZE 			(10) 
 
#define configUSE_APPLICATION_TASK_TAG 		(0)

/***************************************************************** 
 FreeRTOS ���ڴ������й�����ѡ�� 
 *****************************************************************/ 
/* ֧�ֶ�̬�ڴ����� */
#define configSUPPORT_DYNAMIC_ALLOCATION	(1)
/* ֧�־�̬�ڴ� */
#define configSUPPORT_STATIC_ALLOCATION		(0)//ʹ�ö�̬�ڴ�ʱ����Ҫ��Ϊ0
/* ϵͳ�����ܵĶѴ�С */
#define configTOTAL_HEAP_SIZE ((size_t)(36*1024)) 

/*************************************************************** 
FreeRTOS �빳�Ӻ����йص�����ѡ�� 
**************************************************************/ 
/* �� 1��ʹ�ÿ��й��ӣ�Idle Hook �����ڻص����������� 0�����Կ��й��� 
 * 
 * ������������һ������������������û���ʵ�֣� 
 * FreeRTOS �涨�˺��������ֺͲ�����void vApplicationIdleHook(void )�� 
 * ���������ÿ�������������ڶ��ᱻ���� 
 * �����Ѿ�ɾ���� RTOS ���񣬿�����������ͷŷ�������ǵĶ�ջ�ڴ档 
 * ��˱��뱣֤����������Ա� CPU ִ�� 
 * ʹ�ÿ��й��Ӻ������� CPU ����ʡ��ģʽ�Ǻܳ����� 
 * �����Ե��û������������������ API ���� 
 */ 
#define configUSE_IDLE_HOOK					(0)

/* �� 1��ʹ��ʱ��Ƭ���ӣ�Tick Hook������ 0������ʱ��Ƭ���� 
 * 
 * ʱ��Ƭ������һ������������������û���ʵ�֣� 
 * FreeRTOS �涨�˺��������ֺͲ�����void vApplicationTickHook(void ) 
 * ʱ��Ƭ�жϿ��������Եĵ��� 
 * ��������ǳ���С�����ܴ���ʹ�ö�ջ�� 
 * ���ܵ����ԡ�FromISR" �� "FROM_ISR����β�� API ���� 
 */
#define configUSE_TICK_HOOK					(0)

/* ʹ���ڴ�����ʧ�ܹ��Ӻ��� */
#define configUSE_MALLOC_FAILED_HOOK		(0)

/* 
 * ���� 0 ʱ���ö�ջ�����⹦�ܣ����ʹ�ô˹��� 
 * �û������ṩһ��ջ������Ӻ��������ʹ�õĻ� 
 * ��ֵ����Ϊ 1 ���� 2����Ϊ������ջ�����ⷽ�� */
#define configCHECK_FOR_STACK_OVERFLOW		(0)

/******************************************************************** 
 FreeRTOS ������ʱ�������״̬�ռ��йص�����ѡ�� 
 **********************************************************************/ 
/* ��������ʱ��ͳ�ƹ��� */
#define configGENERATE_RUN_TIME_STATS		(0)
/* ���ÿ��ӻ����ٵ��� */
#define configUSE_TRACE_FACILITY			(0)
/* ��� configUSE_TRACE_FACILITY ͬʱΪ 1 ʱ��������� 3 ������ 
 * prvWriteNameToBuffer() 
 * vTaskList(), 
 * vTaskGetRunTimeStats() 
 */ 
#define configUSE_STATS_FORMATTING_FUNCTIONS (1)

/******************************************************************** 
 FreeRTOS ��Э���йص�����ѡ�� 
 *********************************************************************/ 
/* ����Э�̣�����Э���Ժ��������ļ� croutine.c */
#define configUSE_CO_ROUTINES				(0)
/* Э�̵���Ч���ȼ���Ŀ */
#define configMAX_CO_ROUTINE_PRIORITIES		(2)

/*********************************************************************** 
 FreeRTOS �������ʱ���йص�����ѡ�� 
 **********************************************************************/ 
/* ���������ʱ�� */
#define configUSE_TIMERS					(1)
/* �����ʱ�����ȼ� */
#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES-1) 
/* �����ʱ�����г��� */
#define configTIMER_QUEUE_LENGTH			(10)
/* �����ʱ�������ջ��С */
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE*2)

/************************************************************ 
 FreeRTOS ��ѡ��������ѡ�� 
 ************************************************************/
#define INCLUDE_xTaskGetSchedulerState 	(1)
#define INCLUDE_vTaskPrioritySet		(1) 
#define INCLUDE_uxTaskPriorityGet		(1)
#define INCLUDE_vTaskDelete				(1)
#define INCLUDE_vTaskCleanUpResources 	(1)
#define INCLUDE_vTaskSuspend 			(1)
#define INCLUDE_vTaskDelayUntil 		(1) 
#define INCLUDE_vTaskDelay 				(1) 
#define INCLUDE_eTaskGetState 			(1) 
#define INCLUDE_xTimerPendFunctionCall 	(1) 

/****************************************************************** 
 FreeRTOS ���ж��йص�����ѡ�� 
 ******************************************************************/ 
#ifdef __NVIC_PRIO_BITS 
#define configPRIO_BITS __NVIC_PRIO_BITS
#else 
#define configPRIO_BITS	(4)
#endif 
/* �ж�������ȼ� */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY (15)
/* This is the value being used as per the ST library which permits 16
priority values, 0 to 15.  This must correspond to the
configKERNEL_INTERRUPT_PRIORITY setting.  Here 15 corresponds to the lowest
NVIC value of 255. */
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY	(15)

/* ϵͳ�ɹ��������ж����ȼ� */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	(5)
//#define configKERNEL_INTERRUPT_PRIORITY	(255)
#define configKERNEL_INTERRUPT_PRIORITY	(configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
//#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	191
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/**************************************************************** 
 FreeRTOS ���жϷ������йص�����ѡ�� 
 ****************************************************************/ 
#define xPortPendSVHandler 	PendSV_Handler
#define vPortSVCHandler 	SVC_Handler
 
/* ����Ϊʹ�� Percepio Tracealyzer ��Ҫ�Ķ���������Ҫʱ�� configUSE_TRACE_FACILITY ����Ϊ 0 */
#if (configUSE_TRACE_FACILITY == 1)
#include "trcRecorder.h" 
// ����һ����ѡ�������ú����� Trace Դ��ʹ�ã�Ĭ�ϸ�ֵΪ 0 ��ʾ����
#define INCLUDE_xTaskGetCurrentTaskHandle	(1)
#endif

#endif /* FREERTOS_CONFIG_H */
