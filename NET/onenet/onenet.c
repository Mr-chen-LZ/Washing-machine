//单片机头文件
#include "stm32f10x.h"

//网络设备
#include "esp8266.h"

//协议文件
#include "onenet.h"
#include "mqttkit.h"

//硬件驱动
#include "usart.h"
#include "delay.h"

//C库
#include <string.h>
#include <stdio.h>
#include "cJSON.h"
#include "led.h"
#include "oled.h"
#include "beep.h"
#include "Motor.h"
#define PROID		"335e02de4bdc0d92738decbf4599da8b"//设备id

#define AUTH_INFO	"123123"//密码

#define DEVID		"cdk"
cJSON *json,*json_vlaue;

extern unsigned char esp8266_buf[128];

//==========================================================
//	函数名称：	OneNet_DevLink
//
//	函数功能：	与onenet创建连接
//
//	入口参数：	无
//
//	返回参数：	1-成功	0-失败
//
//	说明：		与onenet平台建立连接
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//协议包

	unsigned char *dataPtr;
	
	_Bool status = 1;
	
//	UsartPrintf(USART_DEBUG, "OneNet_DevLink\r\n"
//							"PROID: %s,	AUIF: %s,	DEVID:%s\r\n"
//                        , PROID, AUTH_INFO, DEVID);

	
	if(MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//上传平台
		
		dataPtr = ESP8266_GetIPD(250);									//等待平台响应
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
//					case 0:UsartPrintf(USART_DEBUG, "Tips:	连接成功\r\n");status = 0;break;
					case 0:	OLED_ShowString(0,0,"Tips:OK",20);status = 0;break;
//					case 1:UsartPrintf(USART_DEBUG, "WARN:	连接失败：协议错误\r\n");break;
					case 1:OLED_ShowString(0,0,"ERROR1",20);break;
//					case 2:UsartPrintf(USART_DEBUG, "WARN:	连接失败：非法的clientid\r\n");break;
					case 2:OLED_ShowString(0,0,"ERROR2",20);break;
//					case 3:UsartPrintf(USART_DEBUG, "WARN:	连接失败：服务器失败\r\n");break;
					case 3:OLED_ShowString(0,0,"ERROR3",20);break;
//					case 4:UsartPrintf(USART_DEBUG, "WARN:	连接失败：用户名或密码错误\r\n");break;
					case 4:OLED_ShowString(0,0,"ERROR4",20);break;
//					case 5:UsartPrintf(USART_DEBUG, "WARN:	连接失败：非法链接(比如token非法)\r\n");break;
					case 5:OLED_ShowString(0,0,"ERROR5",20);break;
//					default:UsartPrintf(USART_DEBUG, "ERR:	连接失败：未知错误\r\n");break;
					default:OLED_ShowString(0,0,"ERROR6",20);break;
				}
			}
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//删包
	}
	else
		//UsartPrintf(USART_DEBUG, "WARN:	MQTT_PacketConnect Failed\r\n");
		OLED_ShowString(0,0,"ERROR",20);
	
	return status;
	
}

//==========================================================
//	函数名称：	OneNet_Subscribe
//
//	函数功能：	订阅
//
//	入口参数：	topics：订阅的topic
//				topic_cnt：topic个数
//
//	返回参数：	SEND_TYPE_OK-成功	SEND_TYPE_SUBSCRIBE-需要重发
//
//	说明：		
//==========================================================
void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt)
{
	
	unsigned char i = 0;
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//协议包
	
	for(; i < topic_cnt; i++)
		UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topics[i]);

	
	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL2, topics, topic_cnt, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);					//向平台发送订阅请求
		
		MQTT_DeleteBuffer(&mqttPacket);											//删包
	}

}

//==========================================================
//	函数名称：	OneNet_Publish
//
//	函数功能：	发布消息
//
//	入口参数：	topic：发布的主题
//				msg：消息内容
//
//	返回参数：	SEND_TYPE_OK-成功	SEND_TYPE_PUBLISH-需要重送
//
//	说明：		
//==========================================================
void OneNet_Publish(const char *topic, const char *msg)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//协议包
	
//	UsartPrintf(USART_DEBUG, "Publish Topic: %s, Msg: %s\r\n", topic, msg);
	
	if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL2, 0, 1, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);					//向平台发送订阅请求
		
		MQTT_DeleteBuffer(&mqttPacket);											//删包
	}

}

//==========================================================
//	函数名称：	OneNet_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：		
//==========================================================

void OneNet_RevPro(unsigned char *cmd)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//协议包
	
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
		case MQTT_PKT_CMD:															//命令下发
			
			result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);	//解出topic和消息体
			if(result == 0)
			{
				
				if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)	//命令回复组包
				{			
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//回复命令
					MQTT_DeleteBuffer(&mqttPacket);									//删包
				}
			}
		
		break;

		case MQTT_PKT_PUBLISH:														//接收的Publish消息
		
			result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
			if(result == 0)
			{			
				//解析数据包
				json=cJSON_Parse(req_payload);
				if(!json)
					UsartPrintf(USART_DEBUG,"Error before:[%s]\n",cJSON_GetErrorPtr());
				else
				{
					//解析开关值
					json_vlaue=cJSON_GetObjectItem(json,"target");
					UsartPrintf(USART_DEBUG,"json_vlaue= %d\r\n",json_vlaue->string);//先把键值输出来，就是看看叫什么名字
					UsartPrintf(USART_DEBUG,"json_vlaue= %d\r\n",json_vlaue->valuestring);//接着看看数值是多少
						//从value-int中获取结果
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
			
		case MQTT_PKT_PUBACK:														//发送Publish消息，平台回复的Ack
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Publish Send OK\r\n");
		break;
			
		case MQTT_PKT_PUBREC:														//发送Publish消息，平台回复的Rec，设备需回复Rel消息
		
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
			
		case MQTT_PKT_PUBREL:														//收到Publish消息，设备回复Rec后，平台回复的Rel，设备需再回复Comp
			
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
		
		case MQTT_PKT_PUBCOMP:														//发送Publish消息，平台返回Rec，设备回复Rel，平台再返回的Comp
		
			if(MQTT_UnPacketPublishComp(cmd) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishComp\r\n");
			}
		
		break;
			
		case MQTT_PKT_SUBACK:														//发送Subscribe消息的Ack
		
			if(MQTT_UnPacketSubscribe(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe OK\r\n");
			else
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe Err\r\n");

		break;
			
		case MQTT_PKT_UNSUBACK:														//发送UnSubscribe消息的Ack
		
			if(MQTT_UnPacketUnSubscribe(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe OK\r\n");

			else
				UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe Err\r\n");
		
		break; 
		
		default:
			result = -1;
		break;
	}
	
	ESP8266_Clear();									//清空缓存
	
	if(result == -1)
		return;
	
	dataPtr = strchr(req_payload, '{');					//搜索'}'

	if(dataPtr != NULL && result != -1)					//如果找到了
	{
		dataPtr++;
		
//		while(*dataPtr >= '0' && *dataPtr <= '9')		//判断是否是下发的命令控制数据
//		{
//			numBuf[num++] = *dataPtr++;
//		}
		
//		num = atoi((const char *)numBuf);				//转为数值形式
		num = *dataPtr-'0';
		if(strstr((char *)req_payload, "led"))		//搜索"key"
		{
			LED0 = !num;                   			 //控制LED灯，低电平点亮
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
