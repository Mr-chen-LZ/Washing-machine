#include "key.h"
#include "sys.h" 
#include "delay.h"
//按键初始化函数
void KEY_Init(void) //IO初始化
{ 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);



}

u8 KEY_Scan(void)
{	 
	uint8_t val = 0;
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10) == 0)
	{
		delay_ms(10);
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10) == 0)
			val = 1;
	}
	return val;
}
