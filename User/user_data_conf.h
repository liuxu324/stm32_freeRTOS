/*
 * 用户自定义内容头文件
 */

#ifndef USER_DATA_CONF_H
#define USER_DATA_CONF_H
	
typedef enum task_priority
{
	TASK_PRI_APP = 1,
	TASK_PRI_LED1,	
	TASK_PRI_LED2,
	TASK_PRI_USART,
	TASK_PRI_DS18B20,
	TASK_PRI_CAN,

	TASK_PRI_NUM
} TASK_PRI_E;

#endif /* USER_DATA_CONF_H */

