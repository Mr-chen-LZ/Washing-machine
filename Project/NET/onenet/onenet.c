//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�����豸
#include "esp8266.h"

//Э���ļ�
#include "onenet.h"
#include "mqttkit.h"

//Ӳ������
#include "usart.h"
#include "delay.h"

//C��
#include <string.h>
#include <stdio.h>
#include "cJSON.h"
#include "led.h"
#include "oled.h"
#include "beep.h"
#include "Motor.h"
#define PROID		"335e02de4bdc0d92738decbf4599da8b"//�豸id

#define AUTH_INFO	"123123"//����

#define DEVID		"cdk"
cJSON *json,*json_vlaue;

extern unsigned char esp8266_buf[128];

//==========================================================
//	�������ƣ�	OneNet_DevLink
//
//	�������ܣ�	��onenet��������
//
//	��ڲ�����	��
//
//	���ز�����	1-�ɹ�	0-ʧ��
//
//	˵����		��onenetƽ̨��������
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//Э���

	unsigned char *dataPtr;
	
	_Bool status = 1;
	
//	UsartPrintf(USART_DEBUG, "OneNet_DevLink\r\n"
//							"PROID: %s,	AUIF: %s,	DEVID:%s\r\n"
//                        , PROID, AUTH_INFO, DEVID);

	
	if(MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//�ϴ�ƽ̨
		
		dataPtr = ESP8266_GetIPD(250);									//�ȴ�ƽ̨��Ӧ
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
//					case 0:UsartPrintf(USART_DEBUG, "Tips:	���ӳɹ�\r\n");status = 0;break;
					case 0:	OLED_ShowString(0,0,"Tips:OK",20);status = 0;break;
//					case 1:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�Э�����\r\n");break;
					case 1:OLED_ShowString(0,0,"ERROR1",20);break;
//					case 2:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��Ƿ���clientid\r\n");break;
					case 2:OLED_ShowString(0,0,"ERROR2",20);break;
//					case 3:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�������ʧ��\r\n");break;
					case 3:OLED_ShowString(0,0,"ERROR3",20);break;
//					case 4:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��û������������\r\n");break;
					case 4:OLED_ShowString(0,0,"ERROR4",20);break;
//					case 5:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��Ƿ�����(����token�Ƿ�)\r\n");break;
					case 5:OLED_ShowString(0,0,"ERROR5",20);break;
//					default:UsartPrintf(USART_DEBUG, "ERR:	����ʧ�ܣ�δ֪����\r\n");break;
					default:OLED_ShowString(0,0,"ERROR6",20);break;
				}
			}
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//ɾ��
	}
	else
		//UsartPrintf(USART_DEBUG, "WARN:	MQTT_PacketConnect Failed\r\n");
		OLED_ShowString(0,0,"ERROR",20);
	
	return status;
	
}

//==========================================================
//	�������ƣ�	OneNet_Subscribe
//
//	�������ܣ�	����
//
//	��ڲ�����	topics�����ĵ�topic
//				topic_cnt��topic����
//
//	���ز�����	SEND_TYPE_OK-�ɹ�	SEND_TYPE_SUBSCRIBE-��Ҫ�ط�
//
//	˵����		
//==========================================================
void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt)
{
	
	unsigned char i = 0;
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//Э���
	
	for(; i < topic_cnt; i++)
		UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topics[i]);

	
	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL2, topics, topic_cnt, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);					//��ƽ̨���Ͷ�������
		
		MQTT_DeleteBuffer(&mqttPacket);											//ɾ��
	}

}

//==========================================================
//	�������ƣ�	OneNet_Publish
//
//	�������ܣ�	������Ϣ
//
//	��ڲ�����	topic������������
//				msg����Ϣ����
//
//	���ز�����	SEND_TYPE_OK-�ɹ�	SEND_TYPE_PUBLISH-��Ҫ����
//
//	˵����		
//==========================================================
void OneNet_Publish(const char *topic, const char *msg)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//Э���
	
//	UsartPrintf(USART_DEBUG, "Publish Topic: %s, Msg: %s\r\n", topic, msg);
	
	if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL2, 0, 1, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);					//��ƽ̨���Ͷ�������
		
		MQTT_DeleteBuffer(&mqttPacket);											//ɾ��
	}

}

//==========================================================
//	�������ƣ�	OneNet_RevPro
//
//	�������ܣ�	ƽ̨�������ݼ��
//
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//
//	���ز�����	��
//
//	˵����		
//==========================================================

void OneNet_RevPro(unsigned char *cmd)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//Э���
	
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	
	unsigned short topic_len = 0;
	unsigned short req_len = 0;
	
	unsigned char type = 0;
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;
	
	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;
	
	type = MQTT_UnPacketRecv(cmd);
	switch(type)
	{
		case MQTT_PKT_CMD:															//�����·�
			
			result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);	//���topic����Ϣ��
			if(result == 0)
			{
				
				if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)	//����ظ����
				{			
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//�ظ�����
					MQTT_DeleteBuffer(&mqttPacket);									//ɾ��
				}
			}
		
		break;

		case MQTT_PKT_PUBLISH:														//���յ�Publish��Ϣ
		
			result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
			if(result == 0)
			{			
				//�������ݰ�
				json=cJSON_Parse(req_payload);
				if(!json)
					UsartPrintf(USART_DEBUG,"Error before:[%s]\n",cJSON_GetErrorPtr());
				else
				{
					//��������ֵ
					json_vlaue=cJSON_GetObjectItem(json,"target");
					UsartPrintf(USART_DEBUG,"json_vlaue= %d\r\n",json_vlaue->string);//�ȰѼ�ֵ����������ǿ�����ʲô����
					UsartPrintf(USART_DEBUG,"json_vlaue= %d\r\n",json_vlaue->valuestring);//���ſ�����ֵ�Ƕ���
						//��value-int�л�ȡ���
						if(strstr(json_vlaue->valuestring,"stop")!=NULL)
						{
							json_vlaue=cJSON_GetObjectItem(json,"value");
							if(json_vlaue->valueint)
							{
								OLED_Display_Off();
								Motor_Speed(0);
								GPIO_ResetBits(GPIOA,GPIO_Pin_6);
							}		
						}
						if(strstr(json_vlaue->valuestring,"jishu")!=NULL)
						{
							json_vlaue=cJSON_GetObjectItem(json,"value");
							if(json_vlaue->valueint)
							  Motor_Speed(50); 
							else
								Motor_Speed(30);
						}
							if(strstr(json_vlaue->valuestring,"zanting")!=NULL)
						{
							json_vlaue=cJSON_GetObjectItem(json,"value");
							if(json_vlaue->valueint)
							{
							  Motor_Speed(0); 
							}
							else
								Motor_Speed(30);
						}
							if(strstr(json_vlaue->valuestring,"dianyuan")!=NULL)
						{
							json_vlaue=cJSON_GetObjectItem(json,"value");
							if(json_vlaue->valueint)
							{
								OLED_Display_On();
								OLED_ShowCHineseN(0,2,3,4);
								OLED_ShowCHineseN(0,0,0,3);
							  Motor_Speed(30); 
							}
							else
							{
								OLED_Display_Off();
								Motor_Speed(0);
							}
								
						}
							if(strstr(json_vlaue->valuestring,"xy")!=NULL)
						{
							json_vlaue=cJSON_GetObjectItem(json,"value");
							if(json_vlaue->valueint)
							{
								
							}
							else 
							{
								
							}
								
						}
							if(strstr(json_vlaue->valuestring,"chujun")!=NULL)
						{
							json_vlaue=cJSON_GetObjectItem(json,"value");
							if(json_vlaue->valueint)
							{
								GPIO_SetBits(GPIOA,GPIO_Pin_6);
							}
							else 
							{
								GPIO_ResetBits(GPIOA,GPIO_Pin_6);
							}
								
						}
		
				}
				cJSON_Delete(json);
			}
		
		break;
			
		case MQTT_PKT_PUBACK:														//����Publish��Ϣ��ƽ̨�ظ���Ack
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Publish Send OK\r\n");
		break;
			
		case MQTT_PKT_PUBREC:														//����Publish��Ϣ��ƽ̨�ظ���Rec���豸��ظ�Rel��Ϣ
		
			if(MQTT_UnPacketPublishRec(cmd) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishRec\r\n");
				if(MQTT_PacketPublishRel(MQTT_PUBLISH_ID, &mqttPacket) == 0)
				{
					UsartPrintf(USART_DEBUG, "Tips:	Send PublishRel\r\n");
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);
					MQTT_DeleteBuffer(&mqttPacket);
				}
			}
		
		break;
			
		case MQTT_PKT_PUBREL:														//�յ�Publish��Ϣ���豸�ظ�Rec��ƽ̨�ظ���Rel���豸���ٻظ�Comp
			
			if(MQTT_UnPacketPublishRel(cmd, pkt_id) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishRel\r\n");
				if(MQTT_PacketPublishComp(MQTT_PUBLISH_ID, &mqttPacket) == 0)
				{
					UsartPrintf(USART_DEBUG, "Tips:	Send PublishComp\r\n");
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);
					MQTT_DeleteBuffer(&mqttPacket);
				}
			}
		
		break;
		
		case MQTT_PKT_PUBCOMP:														//����Publish��Ϣ��ƽ̨����Rec���豸�ظ�Rel��ƽ̨�ٷ��ص�Comp
		
			if(MQTT_UnPacketPublishComp(cmd) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishComp\r\n");
			}
		
		break;
			
		case MQTT_PKT_SUBACK:														//����Subscribe��Ϣ��Ack
		
			if(MQTT_UnPacketSubscribe(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe OK\r\n");
			else
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe Err\r\n");

		break;
			
		case MQTT_PKT_UNSUBACK:														//����UnSubscribe��Ϣ��Ack
		
			if(MQTT_UnPacketUnSubscribe(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe OK\r\n");

			else
				UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe Err\r\n");
		
		break; 
		
		default:
			result = -1;
		break;
	}
	
	ESP8266_Clear();									//��ջ���
	
	if(result == -1)
		return;
	
	dataPtr = strchr(req_payload, '{');					//����'}'

	if(dataPtr != NULL && result != -1)					//����ҵ���
	{
		dataPtr++;
		
//		while(*dataPtr >= '0' && *dataPtr <= '9')		//�ж��Ƿ����·��������������
//		{
//			numBuf[num++] = *dataPtr++;
//		}
		
//		num = atoi((const char *)numBuf);				//תΪ��ֵ��ʽ
		num = *dataPtr-'0';
		if(strstr((char *)req_payload, "led"))		//����"key"
		{
			LED0 = !num;                   			 //����LED�ƣ��͵�ƽ����
			MQTT_FreeBuffer(req_payload);
		}
		else if(strstr((char *)req_payload,"beep"))
		{
			if(num)
			{
//				 Beep_Set(BEEP_ON);		
				
			}
			else
			{
//				Beep_Set(BEEP_OFF);
			}
			MQTT_FreeBuffer(req_payload);
		}


		if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
		{
			MQTT_FreeBuffer(cmdid_topic);
			MQTT_FreeBuffer(req_payload);
		}

	}

}
