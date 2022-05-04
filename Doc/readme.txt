/*******************************************
** date：2022-4-19
** add ：can

** date：2022-2-8
** add ：lcd

** date：2022-2-5
** add ：dht11温湿度传感器模块
** add ：ds18b20温度传感器模块
** add ：ds1302实时时钟模块
** add ：hcsr04超声波模块
** add ：mlx90614红外测温模块
** add ：nrf射频模块
** add ：hy2615血压模块
********************************************/

GPIO寄存器描述 《STM32参考手册中文-p75》

1、端口配置低寄存器(GPIOx_CRL)(x = A...E)
2、端口配置高寄存器(GPIOx_CRH)(x = A...E)

3、端口输入数据寄存器(GPIOx_IDR)(x = A...E)
   这些位为只读并只能以字(16位)的形式读出。读出的值为对应I/O口的状态。
   
4、端口输出数据寄存器(GPIOx_ODR)(x = A...E)
   只能以字(16bit)的形式操作，复位值全是0。写0即输出0，写1即输出1。
   
5、端口位设置/清除寄存器(GPIOx_BSRR)(x = A...E)
   高16bit写1用于清0，低16bit写1用于置位，同时写1的话低16bi有效。
   
6、端口位清除寄存器(GPIOx_BRR)(x = A...E)
   低16位写1用于置位。
   
7、端口配置锁定寄存器(GPIOx_LCKR)(x = A...E)

GPIO八种模式
typedef enum
{ 
    GPIO_Mode_AIN = 0x0,            	/* 模拟输入 */   
    GPIO_Mode_IN_FLOATING = 0x04,   	/* 浮空输入，复位后的状态 */
    GPIO_Mode_IPD = 0x28,           	/* 下拉输入 */
    GPIO_Mode_IPU = 0x48,           	/* 上拉输入 */
    GPIO_Mode_Out_OD = 0x14,        	/* 开漏输出 */
    GPIO_Mode_Out_PP = 0x10,        	/* 推挽输出 */
    GPIO_Mode_AF_OD = 0x1C,         	/* 复用开漏输出 */
    GPIO_Mode_AF_PP = 0x18          	/* 复用推挽输出 */
} GPIOMode_TypeDef;