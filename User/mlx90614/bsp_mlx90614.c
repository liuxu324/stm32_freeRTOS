/********************************************************************************
  * @file    调试MLX90614
  * @author  liu xu
  * @date    2017-11-22
  ******************************************************************************/ 
#include "bsp_mlx90614.h"
#include "bsp_SysTick.h"

/*** @brief  MLX90614 初始化 (SMBus总线) ***/
void SMBus_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(SMBUS_CLK, ENABLE);
	
		GPIO_InitStructure.GPIO_Pin = SMBUS_SCK | SMBUS_SDA; 			 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; // 集电极开漏输出  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(SMBUS_PORT, &GPIO_InitStructure);
		
		SMBUS_SCK_H();
		SMBUS_SDA_H();
}

/*******************************************************************************
* Function Name  : SMBus_StartBit(通讯开始信号)
* Description    : Generate START condition on SMBus(SCK=1时，SDA负跳变)
*******************************************************************************/
static void SMBus_StartBit(void)
{
    SMBUS_SDA_H();		// Set SDA line
    SMBUS_SCK_H();		// Set SCL line
    Delay_us(5);	    // Generate bus free time between Stop
    SMBUS_SDA_L();		// Clear SDA line
    Delay_us(5);	    // Hold time after (Repeated) Start
    // Condition. After this period, the first clock is generated.(Thd:sta=4.0us min)
    SMBUS_SCK_L();	  // Clear SCL line
    Delay_us(5);	    
}

/*******************************************************************************
* Function Name  : SMBus_StopBit(通讯停止信号)
* Description    : Generate STOP condition on SMBus(SCK=1时，SDA正跳变)
*******************************************************************************/
static void SMBus_StopBit(void)
{
   	SMBUS_SDA_L();		
    SMBUS_SCK_L();		
	  Delay_us(5);	    
    SMBUS_SCK_H();		// Set SCL line
    Delay_us(5);	    // Stop condition setup time(Tsu:sto=4.0us min)
    SMBUS_SDA_H();		// Set SDA line
}

/*******************************************************************************
* Function Name  : SMBus_WriteBit
* Description    : Write a bit on SMBus 82.5kHz
* Input          : bit_out
*******************************************************************************/
static void SMBus_WriteBit(uint8_t bit_out)
{
    if (bit_out)
    {
        SMBUS_SDA_H();
    }
    else
    {
        SMBUS_SDA_L();
    }
    Delay_us(2);				// Tsu:dat = 250ns minimum
    SMBUS_SCK_H();			// Set SCL line
    Delay_us(6);			  // High Level of Clock Pulse
    SMBUS_SCK_L();			// Clear SCL line
    Delay_us(3);				// Low Level of Clock Pulse
  	SMBUS_SDA_H();	    // Master release SDA line ,
}

/*******************************************************************************
* Function Name  : SMBus_ReadBit
* Description    : Read a bit on SMBus
* Return         : Ack_bit
*******************************************************************************/
static uint8_t SMBus_ReadBit(void)
{
    uint8_t Ack_bit;

    SMBUS_SDA_H();     
	  Delay_us(2);			 // High Level of Clock Pulse
    SMBUS_SCK_H();		
    Delay_us(5);			 // High Level of Clock Pulse
    if (SMBUS_SDA_IN())
        Ack_bit = 1;  
    else
        Ack_bit = 0;
 
    SMBUS_SCK_L();		 // Clear SCL line
    Delay_us(3);			 // Low Level of Clock Pulse

    return	Ack_bit;
}

/*******************************************************************************
* Function Name  : SMBus_ReadByte
* Description    : Read a byte on SMBus(读取一字节数据，高位前、低位后)
* Input          : ack_nack
* Return         : dat(读取的数据)
*******************************************************************************/
static uint8_t SMBus_ReadByte(uint8_t ack_nack)
{
    uint8_t	i = 0;
	  uint8_t dat = 0;

    for (i=0; i<8; i++)
    {
        if (SMBus_ReadBit())  // Get a bit from the SDA line
        {
            dat <<= 1;			
            dat |= 0x01;      // If the bit is HIGH save 1  in dat
        }
        else
        {
            dat <<= 1;			
            dat &= 0xfe;      // If the bit is LOW save 0 in dat
        }
    }
    SMBus_WriteBit(ack_nack); // Write acknowledgment bit
    return dat;
}

/*******************************************************************************
* Function Name  : SMBus_WriteByte
* Description    : Write a byte on SMBus (写字节数据)
* Input          : dat (需要发送的字节)
* Return         : Ack_bit (应答信号状态)
*******************************************************************************/
static uint8_t SMBus_WriteByte(uint8_t dat)
{
    uint8_t	i = 0;
    uint8_t	Ack_bit;
    uint8_t	bit_out;

    for (i=0; i<8; i++)
    {
        if (dat & 0x80)
        {
            bit_out = 1;         // If the current bit of dat is 1 set bit_out
        }
        else
        {
            bit_out = 0;         // else clear bit_out
        }
        SMBus_WriteBit(bit_out); // Send the current bit on SDA
        dat <<= 1;				       // Get next bit for checking
    }

    Ack_bit = SMBus_ReadBit();   // Get acknowledgment bit
    return	Ack_bit;
}

/*******************************************************************************
* Function Name  : PEC_calculation (计算传入数据的包裹校验码--PEC)
* Description    : Calculates the PEC of received bytes
* Input          : pec[]
* Return         : pec[0]-this byte contains calculated crc value
*******************************************************************************/
static uint8_t PEC_Cal(uint8_t pec[])
{
    uint8_t crc[6];
    uint8_t	BitPosition;
    uint8_t	shift;
    uint8_t	i, j;
    uint8_t	temp;
    do
    {
        crc[5] = 0; // Load CRC value 0x000000000107
        crc[4] = 0;
        crc[3] = 0;
        crc[2] = 0;
        crc[1] = 0x01;
        crc[0] = 0x07;

        BitPosition = 47; // Set maximum bit position:47(6 bytes:byte5...byte0,MSbit=47)
        shift = 0;        // Set shift position at 0							     
        i = 5;            // Find first "1" in the transmited message 
        j = 0;            // 在传送的数据中找出第一个“1”
        while ((pec[i]&(0x80>>j))==0 && (i>0))
        {
            BitPosition--;
            if (j<7)
            {
              j++;
            }
            else
            {
              j = 0x00;
              i--;
            }
        }                        // End of while,找出 BitPosition 中为“1”的最高位位置
																 // 得到CRC数值将要移动的位“shift”		
        shift = BitPosition - 8; // Get shift value for pattern value

        while (shift)            // Shift CRC value 
        {                        // 对CRC数据左移“shift”位
            for (i=5; i<0xFF; i--)
            {
                if ((crc[i-1]&0x80) && (i>0))
                {                // 核对字节最高位的下一位是否为“1”
                  temp = 1;      // 是：当前字节 + 1
                }
                else
                {
                  temp = 0;      // 否：当前字节 + 0
                }
                crc[i] <<= 1;
                crc[i] += temp;
            }     
            shift--;
        }         

        for(i=0; i<6; i++)    /*Exclusive OR between pec and crc*/
        {                    
            pec[i] ^= crc[i]; // 在pec and crc 之间进行异或计算
        }/*End of for*/
    }
    while (BitPosition > 8);  /*End of do-while*/

    return pec[0];
}

/*******************************************************************************
 * Function Name  : SMBus_ReadRAM
 * Description    : READ DATA FROM RAM/EEPROM(读RAM中的内容)
 * Input          : saddr(从机地址，7位地址), cmd(存放温度的寄存器地址)
 * Return         : Data
*******************************************************************************/
static uint16_t SMBus_ReadRAM(uint8_t saddr, uint8_t cmd)
{
    uint16_t data = 0;			   	  // Data storage (DataH:DataL)
    uint8_t  DataL = 0;					  // Low data byte storage
    uint8_t  DataH = 0;					  // High data byte storage
    uint8_t  PEC = 0;				 		  // PEC byte storage
    uint8_t  arr[6] = {0};			  // Buffer for the sent bytes(存储已接收数据的缓存)
    uint8_t  PECReg = 0;	        // Calculated PEC byte storage(计算的PEC值)
    uint8_t  ErrorCounter = 10;   // Initialising of ErrorCounter(失败重复次数)
           // Defines the number of the attempts for communication with MLX90614
  	saddr <<= 1;	                // 2-7表示从机地址
    do
    {
			  repeat: 
				SMBus_StopBit();			    //If slave send NACK stop comunication
        
			  ErrorCounter--;				    //Pre-decrement ErrorCounter
        if (!ErrorCounter) 			  //ErrorCounter=0?
            break;					      //Yes,go out from do-while{}
   
        SMBus_StartBit();				  //Start condition
        if (SMBus_WriteByte(saddr))
        {													//Send SlaveAddress 最低位Wr=0表示接下来写命令
          goto repeat;			      //Repeat comunication again
        }
        if (SMBus_WriteByte(cmd)) //Send command
        {
          goto repeat;	  	    	//Repeat comunication again
        }

        SMBus_StartBit();					//Repeated Start condition
																	//Send SlaveAddress 最低位Rd=1表示接下来读数据
        if (SMBus_WriteByte(saddr+1))	
        {
          goto repeat;            //Repeat comunication again
        }

        DataL = SMBus_ReadByte(ACK); //Read low data,master must send ACK
        DataH = SMBus_ReadByte(ACK); //Read high data,master must send ACK
        PEC = SMBus_ReadByte(NACK);	 //Read PEC byte, master must send NACK
        SMBus_StopBit();				     //Stop condition

        arr[5] = saddr;		           //Load array arr
        arr[4] = cmd;			
        arr[3] = saddr + 1;	
        arr[2] = DataL;				    
        arr[1] = DataH;				    
        arr[0] = 0;					      
        PECReg = PEC_Cal(arr);    //Calculate CRC
    }        //If received and calculated CRC are equal go out from do-while{}
    while(PECReg != PEC);		

 	  data = (DataH<<8) | DataL;	  //data=DataH:DataL
    return data;
}

 /*******************************************************************************
 * Function Name  : SMBus_ReadTemp
 * Description    : Calculate and return the temperature
 * Return         : SMBus_ReadMemory(0x00, 0x07)*0.02-273.15
*******************************************************************************/
float SMBus_ReadTemp(void)
{   
	float temp;
	uint8_t i = 0;
	while (5 != i)
	{
		temp += SMBus_ReadRAM(SA, RAM_ACCESS | RAM_TOBJ1)*0.02-273.15;		
		i++;
	}

	return (temp/5.0);	// 平均滤波
}

/*********************************************END OF FILE**********************/
