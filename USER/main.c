/****************************************************************
*��    ��:���������ܰ���ϵͳ
*��    ��:�ź��
*��������:2024/04/21
*֪ ʶ ��:
	1.LVGLͼ�ο��ӻ��������
		(1)LVGL�жϺ���ʵʱ��ʾ��ǰ��ʪ�Ⱥ�RTCʱ��
		(2)LVGL�¼���������RTC������ʱ��	
		(3)LVGL�¼������ص�	
		(4)LVGL�¼���������
		(5)LVGLͼ�ν�����ʾ��ǰ���洫����������������״̬	
	2.RTOS���񴴽���ɾ�������𡢻ָ�
	3.������
		��1�����񻥳����vprintf	
	4.��Ϣ����
		��1�����յ����������͸�esp8266�����ݺ�ʹ����Ϣ���з��͸����ݵ���Ϣ��������
	5.�¼���־��
		��1�������洫���������仯�����Ͷ�Ӧ���¼���־���¼���������
		��2�����������������仯�����Ͷ�Ӧ���¼���־���¼���������
	6.mqttԶ�̿���
		��1�����Ӱ����������������豸����Զ�̿��ƺͼ��
		��2��lot studio��web�˿��ӻ�����		
	7.�ٽ���
		��1���ؼ�����������ı���
		��2�����Ź�ι��	
	8.�����ʱ��
		��1�������ʱ���Ĵ���������
		��2�������ʱ���Ļص�����ʵ�ֶ������Ź�ι������		

*****************************************************************/
#include "includes.h"

//ȫ�ֱ���
u32 flag = 0;
float g_temp=0.0;
float g_humi=0.0;
static volatile uint32_t g_esp8266_init=0;
volatile uint32_t g_system_no_opreation_cnt = 0;

GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
SPI_InitTypeDef SPI_InitStructure;
EXTI_InitTypeDef EXTI_InitStructure;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_DateTypeDef RTC_DateStructure;
RTC_InitTypeDef RTC_InitStructure;

//lvgl�жϺ���
static void timer_cb(lv_timer_t *timer)
{
	static uint32_t t = 0;
	
    char buf[16] = {0};
	
    uint8_t dht11_data[5] = {0};
	
    uint8_t hour;

    // ��ȡ��ǰʱ��
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

    hour = RTC_TimeStructure.RTC_Hours;

    // ����arc�����ν�������
//    lv_arc_set_value(ui_ArcTimeChina, RTC_TimeStructure.RTC_Seconds);

    // ��ʾ����ʱ��
    sprintf(buf, "%02d:%02d:%02d", hour, RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
    lv_label_set_text(ui_LabelTime1, buf);
	lv_label_set_text(ui_LabelTime2, buf);
	lv_label_set_text(ui_LabelTime3, buf);
	
	xEventGroupSetBits(xCreatedEventGroup,0x02);
//	
//	
   t++;

    // ÿ6���ȡ��ʪ��
    if ((t % 6) == 0)
    {
        if (0 == dht11_read_data(dht11_data))
        {
            memset(buf, 0, sizeof buf);

            // �����¶�������ʾ����
            lv_bar_set_value(ui_BarTemperature, dht11_data[2], LV_ANIM_ON);

            // ��ʾ�¶�ֵ
            sprintf(buf, "%d.%d", dht11_data[2], dht11_data[3]);
            lv_label_set_text(ui_LableTemperature, buf);


            memset(buf, 0, sizeof buf);

            // ����ʪ��������ʾ����
            lv_bar_set_value(ui_BarTemHumiture, dht11_data[0], LV_ANIM_ON);

            // ��ʾʪ��ֵ
            sprintf(buf, "%d.%d", dht11_data[0], dht11_data[1]);
            lv_label_set_text(ui_LableHumiture, buf);

        }
    }	

}


/* �����ʱ����� */
static TimerHandle_t soft_timer_Handle = NULL;

/* �����ʱ�� */
static void soft_timer_callback(TimerHandle_t pxTimer);

/* ������ */
TaskHandle_t app_task_init_handle = NULL;
TaskHandle_t app_task_lvgl_handle = NULL;
TaskHandle_t app_task_mq2_handle = NULL;
TaskHandle_t app_task_fire_handle = NULL;
TaskHandle_t app_task_anomaly_handle = NULL;
TaskHandle_t app_task_sr04_handle = NULL;

//mqtt������
static TaskHandle_t g_app_task_mqtt_handle 	  = NULL;
static TaskHandle_t g_app_task_esp8266_handle = NULL;
static TaskHandle_t g_app_task_monitor_handle = NULL;

/* ������*/
static void app_task_init(void *pvParameters);
static void app_task_lvgl(void *pvParameters);
static void app_task_mq2(void *pvParameters);
static void app_task_fire(void *pvParameters);
static void app_task_anomaly(void *pvParameters);
static void app_task_sr04(void *pvParameters);

/* mqtt */  
static void app_task_mqtt(void* pvParameters); 
/* ����WiFiģ��-esp8266*/  
static void app_task_esp8266(void* pvParameters); 
/* �������*/  
static void app_task_monitor(void* pvParameters); 


/* �������ź������ */
static SemaphoreHandle_t g_mutex_printf;

/* �¼���־���� */
EventGroupHandle_t g_event_group;	

/* ��Ϣ���о�� */
QueueHandle_t g_queue_esp8266;

//����¼���
EventGroupHandle_t xCreatedEventGroup;


void dgb_printf_safe(const char *format, ...)
{
#if DEBUG_dgb_printf_safe_EN	

	va_list args;
	va_start(args, format);
	
	/* ��ȡ�����ź��� */
	xSemaphoreTake(g_mutex_printf,portMAX_DELAY);
	
	vprintf(format, args);
			
	/* �ͷŻ����ź��� */
	xSemaphoreGive(g_mutex_printf);	

	va_end(args);
#else
	(void)0;
#endif
}

// ������
int main(void)
{

    /* ����ϵͳ�ж����ȼ�����4 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* ϵͳ��ʱ���ж�Ƶ��ΪconfigTICK_RATE_HZ */
    SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);
	
	/* ���������� */	  
	g_mutex_printf=xSemaphoreCreateMutex();
				  
	/* �����¼���־�� */
	g_event_group=xEventGroupCreate();

	/* ������Ϣ���� */
	g_queue_esp8266=xQueueCreate(3, sizeof(g_esp8266_rx_buf));	
	
	//��������¼���
	xCreatedEventGroup = xEventGroupCreate();

    /* ����app_task_init���� */
    xTaskCreate((TaskFunction_t)app_task_init,          /* ������ں��� */
                (const char *)"app_task_init",          /* �������� */
                (uint16_t)512,                          /* ����ջ��С */
                (void *)NULL,                           /* ������ں������� */
                (UBaseType_t)5,                         /* ��������ȼ� */
                (TaskHandle_t *)&app_task_init_handle); /* ������ƿ�ָ�� */
    /* ����������� */
    vTaskStartScheduler();
				
	while(1)
	{
		//printf("1111111111111111111");
//		delay_ms(1000);
	}
	

	return 0;
}

static void app_task_init(void *pvParameters)
{
	
	led_init();
	
	key_init();
	
	//flash_test();
	
	//spi1_init();
	
	rtc_init();
	
	sr04_init();
	
	dht11_init();
	
   // ��ʼ��lvgl
    lv_init();

    // ��ʼ��lvgl��ʾ�豸
    lv_port_disp_init();

    // ��ʼ��lvgl�����豸
    lv_port_indev_init();

    // ��ʼ��lvgl demo
    //lv_demo_widgets();
	
	ui_init();
	
	usart1_init(115200);
	
	// ��ʱ��3��ʼ��
	tim3_init();	
	
	// ����lvgl��ʱ�����ص�����Ϊtimer_cb����ʱ����Ϊ1000ms
	lv_timer_create(timer_cb, 1000, NULL);	
	
	//��ʼ������
	lv_obj_add_event_cb(ui_TextAreaAdmin, ta_event_cb, LV_EVENT_ALL,ui_Keyboard1);
	lv_obj_add_event_cb(ui_TextAreaPassword, ta_event_cb, LV_EVENT_ALL, ui_Keyboard1);
	lv_obj_add_event_cb(ui_TextAreaEnrllAdmin, ta_event_cb, LV_EVENT_ALL,ui_Keyboard2);
	lv_obj_add_event_cb(ui_TextAreaEnrllpswd, ta_event_cb, LV_EVENT_ALL, ui_Keyboard2);
	lv_obj_add_event_cb(ui_TextAreaEnrllrepswd, ta_event_cb, LV_EVENT_ALL, ui_Keyboard2);
	
	
    /* ����app_task_lvgl���� */
    xTaskCreate((TaskFunction_t)app_task_lvgl,          /* ������ں��� */
                (const char *)"app_task_lvgl",          /* �������� */
                (uint16_t)512,                          /* ����ջ��С */
                (void *)NULL,                           /* ������ں������� */
                (UBaseType_t)8,                         /* ��������ȼ� */
                (TaskHandle_t *)&app_task_lvgl_handle); /* ������ƿ�ָ�� */	
				
    /* ����app_task_mq2���� */
    xTaskCreate((TaskFunction_t)app_task_mq2,          /* ������ں��� */
                (const char *)"app_task_mq2",          /* �������� */
                (uint16_t)512,                          /* ����ջ��С */
                (void *)NULL,                           /* ������ں������� */
                (UBaseType_t)7,                         /* ��������ȼ� */
                (TaskHandle_t *)&app_task_mq2_handle); /* ������ƿ�ָ�� */	
				
    /* ����app_task_fire���� */
    xTaskCreate((TaskFunction_t)app_task_fire,          /* ������ں��� */
                (const char *)"app_task_fire",          /* �������� */
                (uint16_t)512,                          /* ����ջ��С */
                (void *)NULL,                           /* ������ں������� */
                (UBaseType_t)7,                         /* ��������ȼ� */
                (TaskHandle_t *)&app_task_fire_handle); /* ������ƿ�ָ�� */	
				
    /* ����app_task_anomaly���� */
    xTaskCreate((TaskFunction_t)app_task_anomaly,          /* ������ں��� */
                (const char *)"app_task_anomaly",          /* �������� */
                (uint16_t)512,                          /* ����ջ��С */
                (void *)NULL,                           /* ������ں������� */
                (UBaseType_t)7,                         /* ��������ȼ� */
                (TaskHandle_t *)&app_task_anomaly_handle); /* ������ƿ�ָ�� */

	/* ����app_task_mqtt���� */		  
	xTaskCreate((TaskFunction_t )app_task_mqtt,  		/* ������ں��� */
			  (const char*    )"app_task_mqtt",		/* �������� */
			  (uint16_t       )512,  					/* ����ջ��С */
			  (void*          )NULL,					/* ������ں������� */
			  (UBaseType_t    )5, 						/* ��������ȼ� */
			  (TaskHandle_t*  )&g_app_task_mqtt_handle);	/* ������ƿ�ָ�� */	

	/* ����app_task_esp8266���� */		  
	xTaskCreate((TaskFunction_t )app_task_esp8266,  		/* ������ں��� */
			  (const char*    )"app_task_esp8266",		/* �������� */
			  (uint16_t       )1024,  					/* ����ջ��С */
			  (void*          )NULL,					/* ������ں������� */
			  (UBaseType_t    )5, 						/* ��������ȼ� */
			  (TaskHandle_t*  )&g_app_task_esp8266_handle);	/* ������ƿ�ָ�� */				  
			  
	/* ����app_task_monitor���� */		  
	xTaskCreate((TaskFunction_t )app_task_monitor,  		/* ������ں��� */
			  (const char*    )"app_task_monitor",			/* �������� */
			  (uint16_t       )1024,  						/* ����ջ��С */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&g_app_task_monitor_handle);		/* ������ƿ�ָ�� */	
			  
	/* ����app_task_sr04���� */		  
	xTaskCreate((TaskFunction_t )app_task_sr04,  		/* ������ں��� */
			  (const char*    )"app_task_sr04",			/* �������� */
			  (uint16_t       )1024,  						/* ����ջ��С */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_sr04_handle);		/* ������ƿ�ָ�� */	
			  
	/* �������������ʱ�� */
	soft_timer_Handle = xTimerCreate((const char *)"AutoReloadTimer",
									 (TickType_t)1000,	  /* ��ʱ������ 1000(tick) */
									 (UBaseType_t)pdTRUE, /* ����ģʽ */
									 (void *)1,			  /* Ϊÿ����ʱ������һ��������ΨһID */
									 (TimerCallbackFunction_t)soft_timer_callback);
	/* �������������ʱ�� */
	xTimerStart(soft_timer_Handle, 0);
				
				
	vTaskDelete(NULL);
}

static void app_task_lvgl(void *pvParameters)
{
    for (;;)
    {
        //���ڼ����ִ��lvgl������ע�������������(�������Ԫ�ض������¡���ʱ����ˢ�¡�ϵͳ״̬���Ȳ���)
        lv_task_handler();

        //[��ϵͳֻ��1�����񣬿��Բ��ӣ�����������޷�����]˯��5��ʱ�ӽ���
//		xSemaphoreTake(xMutex,portMAX_DELAY);
        vTaskDelay(5);
//		xSemaphoreGive(xMutex);
    }
}

//��������Ӧ
static void app_task_sr04(void *pvParameters)
{
    for (;;)
    {
        
        if(lv_scr_act() == ui_ScreenStart)
		{
			if(get_distance() < 20)
			{
				_ui_screen_change(&ui_ScreenLogin, LV_SCR_LOAD_ANIM_FADE_ON, 150, 0, &ui_ScreenLogin_screen_init);
			}
		}


        vTaskDelay(200);

    }
}

//mq2���
static void app_task_mq2(void *pvParameters)
{
	u32 last_mq2_status = 0;
	u32 mq2_status = 0;
	u8 buf[256];
	
	last_mq2_status = MQ2;
	
	for(;;)
	{
		mq2_status = MQ2;
		if(mq2_status != last_mq2_status)
		{
			memset(buf,0,256);
			sprintf(buf,
				"{\"method\":\"thing.service.property.set\",\"id\":\"0001\",\"params\":{\
			\"Mq2AlarmSwitch\":%d,\
			},\"version\":\"1.0.0\"}",
            (int)MQ2);
			
			//�ϱ���Ϣ��ƽ̨������
			mqtt_publish_data(MQTT_PUBLISH_TOPIC,buf,0);
			
			//��������MQ2���¼�
			xEventGroupSetBitsFromISR(xCreatedEventGroup,EVENT_GROUP_MQ2,NULL);
			
			last_mq2_status = mq2_status;
		}		
			vTaskDelay(50);
			
	}
}

//���洫�������
static void app_task_fire(void *pvParameters)
{
	u32 last_fire_status = 0;
	u32 fire_status = 0;
	u8 buf[256];
	
	last_fire_status = FIRE;
	
	for(;;)
	{
		fire_status = FIRE;
		if(fire_status != last_fire_status)
		{
			memset(buf,0,256);
			sprintf(buf,
				"{\"method\":\"thing.service.property.set\",\"id\":\"0001\",\"params\":{\
			\"FireAlarmSwitch\":%d,\
			},\"version\":\"1.0.0\"}",
            (int)FIRE);
			
			//�ϱ���Ϣ��ƽ̨������
			mqtt_publish_data(MQTT_PUBLISH_TOPIC,buf,0);
			
			//��������FIRE���¼�
			xEventGroupSetBitsFromISR(xCreatedEventGroup,EVENT_GROUP_FIRE,NULL);
			
			last_fire_status = fire_status;
		}		
			vTaskDelay(50);
		//printf("fire : %d\n\r",(int )FIRE);
			
	}

}

//�쳣��������
static void app_task_anomaly(void *pvParameters)
{
	EventBits_t eb;
    for (;;)
    {
		//���յ��¼�
		eb=xEventGroupWaitBits(xCreatedEventGroup,EVENT_GROUP_FIRE|EVENT_GROUP_MQ2,pdTRUE,pdFALSE,portMAX_DELAY);
		
		//���洫�����¼�
		if(eb & EVENT_GROUP_FIRE)
		{
			if(FIRE)
			{
				D4 = 1;
				lv_label_set_text(ui_LabelFireStatus, "fire normal");
				lv_obj_set_style_bg_color(ui_ButtonFireStatus, lv_color_hex(0x28BE54), LV_PART_MAIN | LV_STATE_DEFAULT);
				
			}
			else
			{
				D4 = 0;
				lv_label_set_text(ui_LabelFireStatus, "fire anomaly");
				lv_obj_set_style_bg_color(ui_ButtonFireStatus, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
						
			}

		}

		//mq2�¼�
		if(eb & EVENT_GROUP_MQ2)
		{
			if(MQ2)
			{
			
				D3 = 1;
				lv_label_set_text(ui_LabelMq2Status, "mq2 normal");
				lv_obj_set_style_bg_color(ui_ButtonMq2Status, lv_color_hex(0x28BE54), LV_PART_MAIN | LV_STATE_DEFAULT);
			}
			else
			{
			
				D3 = 0;
				lv_label_set_text(ui_LabelMq2Status, "mq2 anomaly");
				lv_obj_set_style_bg_color(ui_ButtonMq2Status, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);			
			}

		}
		
		vTaskDelay(1);
	
    }
}

static void app_task_mqtt(void* pvParameters)
{
	uint32_t 	delay_1s_cnt=0;
	uint8_t		buf[5]={20,05,56,8,20};
	
	dgb_printf_safe("app_task_mqtt create success\r\n");
	
	dgb_printf_safe("app_task_mqtt suspend\r\n");

	vTaskSuspend(NULL);
	
	dgb_printf_safe("app_task_mqtt resume\r\n");
	
	dht11_init();
	
	vTaskDelay(1000);
	
	for(;;)
	{
		//����������
		mqtt_send_heart();
		
		//�ϱ��豸״̬
		mqtt_report_devices_status();	
		
		delay_ms(1000);
		
		
		delay_1s_cnt++;
		
		
		if(delay_1s_cnt >= 6 )
		{	
			delay_1s_cnt=0;
			
			if(0 == dht11_read_data(buf))
			{
			
				g_temp = (float)buf[2]+(float)buf[3]/10;
				g_humi = (float)buf[0]+(float)buf[1]/10;
			
				printf("Temperature=%.1f Humidity=%.1f\r\n",g_temp,g_humi);
			}

		}
		 
	}
}

static void app_task_monitor(void* pvParameters)
{
	uint32_t esp8266_rx_cnt=0;
	
	BaseType_t xReturn = pdFALSE;	
	
	dgb_printf_safe("app_task_monitor create success \r\n");
	
	for(;;)
	{	
		esp8266_rx_cnt = g_esp8266_rx_cnt;
		
		delay_ms(10);
		
		/* n����󣬷���g_esp8266_rx_cntû�б仯������Ϊ�������ݽ��� */
		if(g_esp8266_init && esp8266_rx_cnt && (esp8266_rx_cnt == g_esp8266_rx_cnt))
		{
			/* ������Ϣ������������ˣ���ʱʱ��Ϊ1000�����ģ����1000�����Ķ�����ʧ�ܣ�����ֱ�ӷ��� */
			xReturn = xQueueSend(g_queue_esp8266,(void *)g_esp8266_rx_buf,1000);		
			
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_monitor] xQueueSend g_queue_esp8266 error code is %d\r\n", xReturn);
			
			g_esp8266_rx_cnt=0;
			memset((void *)g_esp8266_rx_buf,0,sizeof(g_esp8266_rx_buf));
		
		}	
	}
}
static void app_task_esp8266(void* pvParameters)
{
	uint8_t buf[512];
	BaseType_t xReturn = pdFALSE;	
	uint32_t i;
	
	dgb_printf_safe("app_task_esp8266 create success\r\n");
	
	while(esp8266_mqtt_init())
	{
		dgb_printf_safe("esp8266_mqtt_init ...");
		
		delay_ms(1000);
	}
	
	//��������������D1����˸���Σ�ʾ�����ӳɹ�
	beep_on();  PFout(9)=0;delay_ms(100);
	beep_off(); PFout(9)=1;delay_ms(100);	
	beep_on();  PFout(9)=0;delay_ms(100);
	beep_off(); PFout(9)=1;delay_ms(100);

	printf("esp8266 connect aliyun with mqtt success\r\n");	
	
	vTaskResume(g_app_task_mqtt_handle);
	
	g_esp8266_init=1;
	
	for(;;)
	{	
		xReturn = xQueueReceive(g_queue_esp8266,	/* ��Ϣ���еľ�� */
								buf,				/* �õ�����Ϣ���� */
								portMAX_DELAY); 	/* �ȴ�ʱ��һֱ�� */
		if (xReturn != pdPASS)
		{
			dgb_printf_safe("[app_task_esp8266] xQueueReceive error code is %d\r\n", xReturn);
			continue;
		}	

		for(i=0;i<sizeof(buf);i++)
		{
			//�жϵĹؼ��ַ��Ƿ�Ϊ 1"
			//�������ݣ���{"switch_led_1":1}�еġ�1��
			if((buf[i]==0x31) && (buf[i+1]==0x22))
			{
					//�жϿ��Ʊ���
					if( buf[i+3]=='1' )
						PFout(9)=0;//���Ƶ���
					else
						PFout(9)=1;//���Ƶ���
			}	

			//�жϵĹؼ��ַ��Ƿ�Ϊ 2"
			//�������ݣ���{"switch_led_2":1}�еġ�1��
			if((buf[i]==0x32) && (buf[i+1]==0x22))
			{
					//�жϿ��Ʊ���
					if( buf[i+3]=='1' )
						PFout(10)=0;//���Ƶ���
					else
						PFout(10)=1;//���Ƶ���
			}

			//�жϵĹؼ��ַ��Ƿ�Ϊ 3"
			//�������ݣ���{"switch_led_3":1}�еġ�1��
			if(buf[i]==0x33 && buf[i+1]==0x22)
			{
					//�жϿ��Ʊ���
					if( buf[i+3]=='1' )
						PEout(13)=0;//���Ƶ���
					else
						PEout(13)=1;//���Ƶ���
			}				
		}

	}
}

/**
 * @brief �����ʱ������
 * @param pvParameters:��������ʱ���ݵĲ���
 * @retval ��
 * @details ���������������Ź�ι��
 */
static void soft_timer_callback(TimerHandle_t pxTimer)
{
static 
	uint32_t count_backwards_sta=0;
//	oled_t oled;
//	BaseType_t xReturn;
	uint8_t buf[16] = {0};
	int32_t timer_id = (int32_t)pvTimerGetTimerID(pxTimer);

	/* ����״̬���� �� ͳ��ϵͳ�޲�������ֵΪ0 �� g_system_no_opreation_cnt�ﵽϨ��ʱ��ֵ */
	if (count_backwards_sta && (g_system_no_opreation_cnt == 0 || g_system_no_opreation_cnt >= SCREEN_OFF_TIME))
	{
		/* ��ռ�¼����״̬ */
		count_backwards_sta = 0;


		memset(buf, 0, sizeof buf);
		sprintf((char *)buf, "%d", SCREEN_OFF_TIME - g_system_no_opreation_cnt);


	}

	/* ��ʼ����,3 2 1 */
	if ((g_system_no_opreation_cnt >= SCREEN_OFF_TIME - 3) && (g_system_no_opreation_cnt < SCREEN_OFF_TIME))
	{
		dgb_printf_safe("[soft_timer_callback id=%d] g_system_no_opreation_cnt is %d\r\n", timer_id, g_system_no_opreation_cnt);

		/* ��¼����״̬ */
		count_backwards_sta=1;

		memset(buf, 0, sizeof buf);
		sprintf((char *)buf, "%d", SCREEN_OFF_TIME - g_system_no_opreation_cnt);

	}

	/* g_system_no_opreation_cnt�ﵽϨ��ʱ��ֵ����ִ��OLEDϨ��*/
	if (g_system_no_opreation_cnt >= SCREEN_OFF_TIME)
	{
		
	}

	/* �����ٽ�����FreeRTOSͬʱ��ر��жϣ�UCOS��OS_CRITICAL_ENTER��ֹͣ������ȣ� */
	taskENTER_CRITICAL();

#if IWDG_ENABLE
	/* ι����ˢ���������ֵ */
	IWDG_ReloadCounter();
#endif
	/* ͳ��ϵͳ�޲�������ֵ�Լ�1 */
	g_system_no_opreation_cnt++;

	/* �˳��ٽ�����FreeRTOSͬʱ����жϣ�UCOS��OS_CRITICAL_EXTI��ָ�������ȣ� */
	taskEXIT_CRITICAL();
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}


void vApplicationTickHook( void )
{
//	lv_tick_inc(1);
}
