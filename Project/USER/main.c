#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "led.h"	
#include "key.h"	
#include "esp8266.h"
#include "onenet.h"
#include "mqttkit.h"
#include "beep.h"	
#include "oled.h"
#include "bmp.h"
#include "dht11.h"
#include "adc.h"
#include "Motor.h"
#include "stdio.h"
#include "string.h"

int8_t PUB_BUF[256];//�ϴ����ݵ�buf
const uint8_t *topics[] = {"/iot/6273/cdk"};
uint16_t timeCount = 0;	//���ͼ������
uint8_t *dataPtr = NULL;
int8_t Speed = 30;
int8_t oled_Flag = 1;
uint16_t temp_max = 0;
uint16_t humi_max = 0;
uint16_t light_max = 0;
uint8_t alarmTemp_flag = 1;
uint8_t alarmHumi_flag = 1;
uint8_t alarmLight_flag = 1;
uint8_t key = 0;
uint8_t key1 = 0;
uint8_t mode = 0;
uint8_t temp,humi;
uint16_t adcx;														  //adc�ɼ�����


void show_Porc(void);
void data_Poce(void);
void key_Poce(void);

int main(void)
{	
	NVIC_Configuration(); 	 							//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	delay_init();	    	 								  //��ʱ������ʼ��	 
	Usart1_Init(115200);								  //8266ͨѶ���ڳ�ʼ��
	Usart2_Init(115200);								  //DEBUG���ڲ�δʹ�ã��жϹر�
	OLED_Init();													//OLED��ʼ��
	Beep_Init();													//��������ʼ��
	LED_Init();													  //LED��ʼ��
	Adc_Init();													  //ADC��ʼ��
	KEY_Init();													  //������ʼ��
	Motor_Init();                         //ֱ�������ʼ��
	while(DHT11_Init())									  //DHT11��ʼ��
	{
		OLED_ShowString(0,0,"DHT11 init error!",20);		 
		delay_ms(500);
	}						 
	ESP8266_Init();												//8266��ʼ��
	while(OneNet_DevLink())								//����onenet
		
	delay_ms(500);
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);										//���н���ɹ�
	delay_ms(500);
	GPIO_SetBits(GPIOA,GPIO_Pin_12);
	
	OneNet_Subscribe(topics, 1);					//MQTT���Ļ���
	OLED_Clear();					

	OLED_ShowCHineseN(64,0,18,1);
	OLED_ShowCHineseN(0,2,3,3);
	OLED_ShowString(64,2,"%RH",16);
	OLED_ShowCHineseN(0,4,6,5);
	  OLED_ShowCHinese(0,0,0);
	OLED_ShowString(104,4,"lux",16);

	while(1) 
	{	
		key_Poce();
		data_Poce();
		show_Porc();
	}
}

void show_Porc(void)
{
	switch(mode)										//OLED��ʾ
	{
		case 0:
		{
			OLED_ShowCHineseN(16,6,0,2);
			OLED_ShowCHineseN(48,6,23,2);
			OLED_ShowNum(80,6,temp_max,2,16);
			OLED_ShowCHineseN(96,6,18,1);
			OLED_ShowCHineseN(112,6,27,1);
			
			if(alarmTemp_flag == 0)	//����ָʾͼ��
				OLED_ShowCHineseN(0,6,26,1);
			else
				OLED_ShowCHineseN(0,6,28,1);		
			
		}
		break;
		case 1:
		{
			OLED_ShowCHineseN(16,6,3,2);
			OLED_ShowCHineseN(48,6,23,2);
			OLED_ShowNum(80,6,humi_max,2,16);
			OLED_ShowString(96,6,"%RH ",16);
			
			if(alarmHumi_flag == 0)	//����ָʾͼ��
				OLED_ShowCHineseN(0,6,26,1);
			else
				OLED_ShowCHineseN(0,6,28,1);
			
		}
		break;
		default:
		{
			OLED_ShowCHineseN(16,6,6,1);
			OLED_ShowCHineseN(32,6,8,1);
			OLED_ShowCHineseN(48,6,23,2);
			OLED_ShowNum(80,6,light_max,4,16);
			OLED_ShowString(112,6,"lu",16);
			
			if(alarmLight_flag ==0)	//����ָʾͼ��
				OLED_ShowCHineseN(0,6,26,1);
			else
				OLED_ShowCHineseN(0,6,28,1);
			
		}
		break;
	}
}

void data_Poce(void)
{
		if(oled_Flag && (timeCount ==50))
		{
			oled_Flag = 0;
			sprintf((char *)PUB_BUF,"{\"light\":%d,\"temp\":%d,\"humi\":%d,\"oled_gata\":%d}",adcx,temp,humi,1);				//�ϴ���MQTT������
			OneNet_Publish("/iot/6273/cc",(const char *)PUB_BUF);
			ESP8266_Clear();
		}
		if(timeCount%100 == 1)
		{
			DHT11_Read_Data(&temp,&humi);													//��ȡ��ʪ��
      OLED_ShowNum(48,0,temp,2,16);
      OLED_ShowNum(48,2,humi,2,16);
			adcx=Get_Adc_Average(ADC_Channel_0,1);								//ADC��ȡ����ǿ��
      OLED_ShowNum(72,4,adcx,4,16);

			
		}	
		if(++timeCount >= 500)
		{
			timeCount=0;
			sprintf((char *)PUB_BUF,"{\"light\":%d,\"temp\":%d,\"humi\":%d}",adcx,temp,humi);				//�ϴ���MQTT������
			OneNet_Publish("/iot/6273/cc",(const char *)PUB_BUF);
			ESP8266_Clear();
		}
		dataPtr = ESP8266_GetIPD(1);														//�鿴��������
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);
		delay_ms(10);  
		
			
		if(alarmTemp_flag == 0)					//�¶ȱ����ж�
		{
			if(temp >= temp_max)
			{
				GPIO_ResetBits(GPIOA,GPIO_Pin_12);	
			}
			else
			{
				GPIO_SetBits(GPIOA,GPIO_Pin_12);
			}
		}

		if(alarmHumi_flag == 0)			//ʪ�ȱ����ж�
		{
			if(humi >= humi_max)
			{
				GPIO_ResetBits(GPIOA,GPIO_Pin_12);
			}
			else
			{
				GPIO_SetBits(GPIOA,GPIO_Pin_12);
			}			
		}

		if(alarmLight_flag == 0)  //����ǿ�ȱ����ж�
		{
			if(adcx >= light_max)
			{
				GPIO_ResetBits(GPIOA,GPIO_Pin_12);
			}
			else
			{
				GPIO_SetBits(GPIOA,GPIO_Pin_12);
			}			
		}
}

void key_Poce(void)
{
	key1 = KEY_Scan();						//����ɨ�躯��
	if(key != key1)
		key = key1;
	else
		key1 = 0;
		
	if(key1 == 1)
		{
			 Motor_Speed(Speed); 
			 sprintf((char *)PUB_BUF,"{\"light\":%d,\"temp\":%d,\"humi\":%d,\"dianyuan\":%d}",adcx,temp,humi,1);				//�ϴ���MQTT������
			 OneNet_Publish("/iot/6273/cc",(const char *)PUB_BUF);
			 ESP8266_Clear();
			 OLED_ShowCHineseN(0,2,3,4);
			 OLED_ShowCHineseN(0,0,0,3);
	 }
}

