/*******************************************************************************
  * @file    bsp_key.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   keyӦ�ú����ӿ�
  ******************************************************************************
  */

/*������ʱ������˵����
 *1.������ʱһ��Ϊ10ms
 *2.ʵ�ʹ����У�������ѯ��ⲻ����main()��while(1)ѭ���н��У�
 	�磺	if (KEY_ON == Key_Scan(GPIOA, GPIO_Pin_0))
		{
			LED1_TOGGLE;
		}
 *3.���Ƿ����жϷ������д���������ִ�е�Ч��
 */
  
#include "bsp_key.h"   
#include "bsp_SysTick.h"

/**
* @brief ����Ƿ��а�������
* @param ����Ķ˿ںͶ˿�λ
* @arg GPIOx: x �����ǣ�A...G��
* @arg GPIO_PIN ������ GPIO_PIN_x��x ������ 1...16��
* @retval ������״̬
* @arg KEY_ON:��������
* @arg KEY_OFF:����û����
*/
uint8_t Key_Scan(GPIO_TypeDef* GPIOx, u16 GPIO_Pin)
{
	/*����Ƿ��а������� */
	if (GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == KEY_ON ) 
	{
		/*��ʱ����,10ms*/
		Delay_us(10*1000);
		if (GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == KEY_ON ) 
		{
			/*�ȴ������ͷ� */
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
  * @brief  ��ʼ������KEY��IO
  * @param  ��
  * @retval ��
  */
void KEY_GPIO_Config(void)
{		
	/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*����GPIOA��GPIOC������ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE); 

	/*ѡ��Ҫ���Ƶ�GPIOA����*/															   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	

	/*��������ģʽΪ��������*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   

	/*������������Ϊ50MHz */   
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*���ÿ⺯������ʼ��GPIOA0*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	/*ѡ��Ҫ���Ƶ�GPIOC����*/															   
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;

	/*���ÿ⺯������ʼ��GPIOC13*/
	//GPIO_Init(GPIOC, &GPIO_InitStructure);	 
}
/*********************************************END OF FILE**********************/

