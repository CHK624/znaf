#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"

#include "sys.h"

//ϵͳ
#include "usart.h"
#include "delay.h"

//����
#include "esp8266.h"
#include "esp8266_mqtt.h"
#include "sr04.h"
#include "led.h"
#include <mq2.h>
#include <fire.h>
#include "beep.h"
#include "key.h"
#include "tim.h"
#include "tft.h"
#include "bmp.h"
#include "touch.h"  
#include "rtc.h"
#include "dht11.h"
#include "iwdg.h"

//freertos
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

//lvgl
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "ui.h"
#include "ui_helpers.h"

void lvgl_masbox_enroll(u32 check);

//�л�����
void ta_event_cb(lv_event_t * e);

/* ���� */
extern float g_temp;
extern float g_humi;
extern volatile u8 client[10][32];
extern volatile u32 client_num;

extern GPIO_InitTypeDef  	GPIO_InitStructure;
extern NVIC_InitTypeDef 	NVIC_InitStructure;		
extern SPI_InitTypeDef  	SPI_InitStructure;

extern EXTI_InitTypeDef EXTI_InitStructure;
extern RTC_TimeTypeDef RTC_TimeStructure;
extern RTC_DateTypeDef RTC_DateStructure;
extern RTC_InitTypeDef RTC_InitStructure;

/* �������ź������ */
extern SemaphoreHandle_t g_mutex_printf;


/* �¼���־���� */
extern EventGroupHandle_t g_event_group;	

/* ��Ϣ���о�� */
extern QueueHandle_t 	g_queue_esp8266;


//����¼���
extern EventGroupHandle_t xCreatedEventGroup;


//�궨��
#define EVENT_GROUP_MQ2 0x01
#define EVENT_GROUP_FIRE 0x02

#define SCREEN_OFF_TIME				60
#define DEBUG_dgb_printf_safe_EN	1


#endif
