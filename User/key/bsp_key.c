/*******************************************************************************
  * @file    bsp_key.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   key应用函数接口
  ******************************************************************************
  */

/*关于延时消抖的说明：
 *1.消抖延时一般为10ms
 *2.实际工作中，按键轮询检测不是在main()的while(1)循环中进行，
 	如：	if (KEY_ON == Key_Scan(GPIOA, GPIO_Pin_0))
		{
			LED1_TOGGLE;
		}
 *3.而是放在中断服务函数中处理，提高软件执行的效率
 */
  
#include "bsp_key.h"   
#include "bsp_SysTick.h"

/**
* @brief 检测是否有按键按下
* @param 具体的端口和端口位
* @arg GPIOx: x 可以是（A...G）
* @arg GPIO_PIN 可以是 GPIO_PIN_x（x 可以是 1...16）
* @retval 按键的状态
* @arg KEY_ON:按键按下
* @arg KEY_OFF:按键没按下
*/
uint8_t Key_Scan(GPIO_TypeDef* GPIOx, u16 GPIO_Pin)
{
	/*检测是否有按键按下 */
	if (GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == KEY_ON ) 
	{
		/*延时消抖,10ms*/
		Delay_us(10*1000);
		if (GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == KEY_ON ) 
		{
			/*等待按键释放 */
			while (GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == KEY_ON)
			 	;

			return KEY_ON;
		} 
		else
		{
			return KEY_OFF;
		}	 
	} 
	else
	{
		return KEY_OFF;
	}
}

/**
  * @brief  初始化控制KEY的IO
  * @param  无
  * @retval 无
  */
void KEY_GPIO_Config(void)
{		
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*开启GPIOA和GPIOC的外设时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE); 

	/*选择要控制的GPIOA引脚*/															   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	

	/*设置引脚模式为上拉输入*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   

	/*设置引脚速率为50MHz */   
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*调用库函数，初始化GPIOA0*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	/*选择要控制的GPIOC引脚*/															   
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;

	/*调用库函数，初始化GPIOC13*/
	//GPIO_Init(GPIOC, &GPIO_InitStructure);	 
}
/*********************************************END OF FILE**********************/

