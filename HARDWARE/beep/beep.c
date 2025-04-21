//单片机头文件
#include "stm32f10x.h"

//硬件驱动
#include "beep.h"



//BEEP_INFO beepInfo = {0};


/*
************************************************************
*	函数名称：	Beep_Init
*
*	函数功能：	蜂鸣器初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Beep_Init(void)
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		
	
	GPIO_InitTypeDef gpioInitStruct;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;				//设置为输出
	gpioInitStruct.GPIO_Pin = GPIO_Pin_12;						//将初始化的Pin脚
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;				//可承载的最大频率
	GPIO_Init(GPIOB, &gpioInitStruct);							//初始化GPIO
	
  GPIO_SetBits(GPIOA,GPIO_Pin_12);

}
