#ifndef __DHT11_H
#define __DHT11_H 
#include "sys.h"   
 
//IO��������
#define DHT11_IO_IN()  {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=8<<16;}//�ڰ�λΪ1
#define DHT11_IO_OUT() {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=3<<16;}//������Ϊ1
////IO��������											   
#define	DHT11_DQ_OUT PAout(4) //���ݶ˿�	PA1
#define	DHT11_DQ_IN  PAin(4)  //���ݶ˿�	PA1



u8 DHT11_Init(void);//��ʼ��DHT11
u8 DHT11_Read_Data(u8 *temp,u8 *humi);//��ȡ��ʪ��
u8 DHT11_Read_Byte(void);//����һ���ֽ�
u8 DHT11_Read_Bit(void);//����һ��λ
u8 DHT11_Check(void);//����Ƿ����DHT11
void DHT11_Rst(void);//��λDHT11    
#endif















