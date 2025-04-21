//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//Ӳ������
#include "beep.h"



//BEEP_INFO beepInfo = {0};


/*
************************************************************
*	�������ƣ�	Beep_Init
*
*	�������ܣ�	��������ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void Beep_Init(void)
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		
	
	GPIO_InitTypeDef gpioInitStruct;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;				//����Ϊ���
	gpioInitStruct.GPIO_Pin = GPIO_Pin_12;						//����ʼ����Pin��
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;				//�ɳ��ص����Ƶ��
	GPIO_Init(GPIOB, &gpioInitStruct);							//��ʼ��GPIO
	
  GPIO_SetBits(GPIOA,GPIO_Pin_12);

}
