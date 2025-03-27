/****************************************************************
*名    称:阿里云智能安防系统
*作    者:古弘驰
*创建日期:2024/04/21
*知 识 点:
	1.LVGL图形可视化界面设计
		(1)LVGL中断函数实时显示当前温湿度和RTC时间
		(2)LVGL事件函数设置RTC日期与时间	
		(3)LVGL事件函数控灯	
		(4)LVGL事件函数锁屏
		(5)LVGL图形界面显示当前火焰传感器和烟雾传感器的状态	
	2.RTOS任务创建、删除、挂起、恢复
	3.互斥锁
		（1）任务互斥访问vprintf	
	4.消息队列
		（1）接收到物联网发送给esp8266的数据后，使用消息队列发送该数据到消息处理任务
	5.事件标志组
		（1）当火焰传感器发生变化，发送对应的事件标志给事件处理任务
		（2）当烟雾传感器发生变化，发送对应的事件标志给事件处理任务
	6.mqtt远程控制
		（1）连接阿里云物联网，对设备进行远程控制和监控
		（2）lot studio的web端可视化界面		
	7.临界区
		（1）关键变量、代码的保护
		（2）看门狗喂狗	
	8.软件定时器
		（1）软件定时器的创建与配置
		（2）软件定时器的回调函数实现独立看门狗喂狗操作		

*****************************************************************/
#include "includes.h"

//全局变量
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

//lvgl中断函数
static void timer_cb(lv_timer_t *timer)
{
	static uint32_t t = 0;
	
    char buf[16] = {0};
	
    uint8_t dht11_data[5] = {0};
	
    uint8_t hour;

    // 获取当前时间
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

    hour = RTC_TimeStructure.RTC_Hours;

    // 设置arc（弧形进度条）
//    lv_arc_set_value(ui_ArcTimeChina, RTC_TimeStructure.RTC_Seconds);

    // 显示北京时间
    sprintf(buf, "%02d:%02d:%02d", hour, RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
    lv_label_set_text(ui_LabelTime1, buf);
	lv_label_set_text(ui_LabelTime2, buf);
	lv_label_set_text(ui_LabelTime3, buf);
	
	xEventGroupSetBits(xCreatedEventGroup,0x02);
//	
//	
   t++;

    // 每6秒获取温湿度
    if ((t % 6) == 0)
    {
        if (0 == dht11_read_data(dht11_data))
        {
            memset(buf, 0, sizeof buf);

            // 设置温度条的显示长度
            lv_bar_set_value(ui_BarTemperature, dht11_data[2], LV_ANIM_ON);

            // 显示温度值
            sprintf(buf, "%d.%d", dht11_data[2], dht11_data[3]);
            lv_label_set_text(ui_LableTemperature, buf);


            memset(buf, 0, sizeof buf);

            // 设置湿度条的显示长度
            lv_bar_set_value(ui_BarTemHumiture, dht11_data[0], LV_ANIM_ON);

            // 显示湿度值
            sprintf(buf, "%d.%d", dht11_data[0], dht11_data[1]);
            lv_label_set_text(ui_LableHumiture, buf);

        }
    }	

}


/* 软件定时器句柄 */
static TimerHandle_t soft_timer_Handle = NULL;

/* 软件定时器 */
static void soft_timer_callback(TimerHandle_t pxTimer);

/* 任务句柄 */
TaskHandle_t app_task_init_handle = NULL;
TaskHandle_t app_task_lvgl_handle = NULL;
TaskHandle_t app_task_mq2_handle = NULL;
TaskHandle_t app_task_fire_handle = NULL;
TaskHandle_t app_task_anomaly_handle = NULL;
TaskHandle_t app_task_sr04_handle = NULL;

//mqtt任务句柄
static TaskHandle_t g_app_task_mqtt_handle 	  = NULL;
static TaskHandle_t g_app_task_esp8266_handle = NULL;
static TaskHandle_t g_app_task_monitor_handle = NULL;

/* 任务函数*/
static void app_task_init(void *pvParameters);
static void app_task_lvgl(void *pvParameters);
static void app_task_mq2(void *pvParameters);
static void app_task_fire(void *pvParameters);
static void app_task_anomaly(void *pvParameters);
static void app_task_sr04(void *pvParameters);

/* mqtt */  
static void app_task_mqtt(void* pvParameters); 
/* 无线WiFi模块-esp8266*/  
static void app_task_esp8266(void* pvParameters); 
/* 监控任务*/  
static void app_task_monitor(void* pvParameters); 


/* 互斥型信号量句柄 */
static SemaphoreHandle_t g_mutex_printf;

/* 事件标志组句柄 */
EventGroupHandle_t g_event_group;	

/* 消息队列句柄 */
QueueHandle_t g_queue_esp8266;

//监控事件组
EventGroupHandle_t xCreatedEventGroup;


void dgb_printf_safe(const char *format, ...)
{
#if DEBUG_dgb_printf_safe_EN	

	va_list args;
	va_start(args, format);
	
	/* 获取互斥信号量 */
	xSemaphoreTake(g_mutex_printf,portMAX_DELAY);
	
	vprintf(format, args);
			
	/* 释放互斥信号量 */
	xSemaphoreGive(g_mutex_printf);	

	va_end(args);
#else
	(void)0;
#endif
}

// 主函数
int main(void)
{

    /* 设置系统中断优先级分组4 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* 系统定时器中断频率为configTICK_RATE_HZ */
    SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);
	
	/* 创建互斥锁 */	  
	g_mutex_printf=xSemaphoreCreateMutex();
				  
	/* 创建事件标志组 */
	g_event_group=xEventGroupCreate();

	/* 创建消息队列 */
	g_queue_esp8266=xQueueCreate(3, sizeof(g_esp8266_rx_buf));	
	
	//创建监控事件组
	xCreatedEventGroup = xEventGroupCreate();

    /* 创建app_task_init任务 */
    xTaskCreate((TaskFunction_t)app_task_init,          /* 任务入口函数 */
                (const char *)"app_task_init",          /* 任务名字 */
                (uint16_t)512,                          /* 任务栈大小 */
                (void *)NULL,                           /* 任务入口函数参数 */
                (UBaseType_t)5,                         /* 任务的优先级 */
                (TaskHandle_t *)&app_task_init_handle); /* 任务控制块指针 */
    /* 开启任务调度 */
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
	
   // 初始化lvgl
    lv_init();

    // 初始化lvgl显示设备
    lv_port_disp_init();

    // 初始化lvgl输入设备
    lv_port_indev_init();

    // 初始化lvgl demo
    //lv_demo_widgets();
	
	ui_init();
	
	usart1_init(115200);
	
	// 定时器3初始化
	tim3_init();	
	
	// 创建lvgl定时器，回调函数为timer_cb，定时周期为1000ms
	lv_timer_create(timer_cb, 1000, NULL);	
	
	//初始化键盘
	lv_obj_add_event_cb(ui_TextAreaAdmin, ta_event_cb, LV_EVENT_ALL,ui_Keyboard1);
	lv_obj_add_event_cb(ui_TextAreaPassword, ta_event_cb, LV_EVENT_ALL, ui_Keyboard1);
	lv_obj_add_event_cb(ui_TextAreaEnrllAdmin, ta_event_cb, LV_EVENT_ALL,ui_Keyboard2);
	lv_obj_add_event_cb(ui_TextAreaEnrllpswd, ta_event_cb, LV_EVENT_ALL, ui_Keyboard2);
	lv_obj_add_event_cb(ui_TextAreaEnrllrepswd, ta_event_cb, LV_EVENT_ALL, ui_Keyboard2);
	
	
    /* 创建app_task_lvgl任务 */
    xTaskCreate((TaskFunction_t)app_task_lvgl,          /* 任务入口函数 */
                (const char *)"app_task_lvgl",          /* 任务名字 */
                (uint16_t)512,                          /* 任务栈大小 */
                (void *)NULL,                           /* 任务入口函数参数 */
                (UBaseType_t)8,                         /* 任务的优先级 */
                (TaskHandle_t *)&app_task_lvgl_handle); /* 任务控制块指针 */	
				
    /* 创建app_task_mq2任务 */
    xTaskCreate((TaskFunction_t)app_task_mq2,          /* 任务入口函数 */
                (const char *)"app_task_mq2",          /* 任务名字 */
                (uint16_t)512,                          /* 任务栈大小 */
                (void *)NULL,                           /* 任务入口函数参数 */
                (UBaseType_t)7,                         /* 任务的优先级 */
                (TaskHandle_t *)&app_task_mq2_handle); /* 任务控制块指针 */	
				
    /* 创建app_task_fire任务 */
    xTaskCreate((TaskFunction_t)app_task_fire,          /* 任务入口函数 */
                (const char *)"app_task_fire",          /* 任务名字 */
                (uint16_t)512,                          /* 任务栈大小 */
                (void *)NULL,                           /* 任务入口函数参数 */
                (UBaseType_t)7,                         /* 任务的优先级 */
                (TaskHandle_t *)&app_task_fire_handle); /* 任务控制块指针 */	
				
    /* 创建app_task_anomaly任务 */
    xTaskCreate((TaskFunction_t)app_task_anomaly,          /* 任务入口函数 */
                (const char *)"app_task_anomaly",          /* 任务名字 */
                (uint16_t)512,                          /* 任务栈大小 */
                (void *)NULL,                           /* 任务入口函数参数 */
                (UBaseType_t)7,                         /* 任务的优先级 */
                (TaskHandle_t *)&app_task_anomaly_handle); /* 任务控制块指针 */

	/* 创建app_task_mqtt任务 */		  
	xTaskCreate((TaskFunction_t )app_task_mqtt,  		/* 任务入口函数 */
			  (const char*    )"app_task_mqtt",		/* 任务名字 */
			  (uint16_t       )512,  					/* 任务栈大小 */
			  (void*          )NULL,					/* 任务入口函数参数 */
			  (UBaseType_t    )5, 						/* 任务的优先级 */
			  (TaskHandle_t*  )&g_app_task_mqtt_handle);	/* 任务控制块指针 */	

	/* 创建app_task_esp8266任务 */		  
	xTaskCreate((TaskFunction_t )app_task_esp8266,  		/* 任务入口函数 */
			  (const char*    )"app_task_esp8266",		/* 任务名字 */
			  (uint16_t       )1024,  					/* 任务栈大小 */
			  (void*          )NULL,					/* 任务入口函数参数 */
			  (UBaseType_t    )5, 						/* 任务的优先级 */
			  (TaskHandle_t*  )&g_app_task_esp8266_handle);	/* 任务控制块指针 */				  
			  
	/* 创建app_task_monitor任务 */		  
	xTaskCreate((TaskFunction_t )app_task_monitor,  		/* 任务入口函数 */
			  (const char*    )"app_task_monitor",			/* 任务名字 */
			  (uint16_t       )1024,  						/* 任务栈大小 */
			  (void*          )NULL,						/* 任务入口函数参数 */
			  (UBaseType_t    )5, 							/* 任务的优先级 */
			  (TaskHandle_t*  )&g_app_task_monitor_handle);		/* 任务控制块指针 */	
			  
	/* 创建app_task_sr04任务 */		  
	xTaskCreate((TaskFunction_t )app_task_sr04,  		/* 任务入口函数 */
			  (const char*    )"app_task_sr04",			/* 任务名字 */
			  (uint16_t       )1024,  						/* 任务栈大小 */
			  (void*          )NULL,						/* 任务入口函数参数 */
			  (UBaseType_t    )5, 							/* 任务的优先级 */
			  (TaskHandle_t*  )&app_task_sr04_handle);		/* 任务控制块指针 */	
			  
	/* 创建周期软件定时器 */
	soft_timer_Handle = xTimerCreate((const char *)"AutoReloadTimer",
									 (TickType_t)1000,	  /* 定时器周期 1000(tick) */
									 (UBaseType_t)pdTRUE, /* 周期模式 */
									 (void *)1,			  /* 为每个计时器分配一个索引的唯一ID */
									 (TimerCallbackFunction_t)soft_timer_callback);
	/* 开启周期软件定时器 */
	xTimerStart(soft_timer_Handle, 0);
				
				
	vTaskDelete(NULL);
}

static void app_task_lvgl(void *pvParameters)
{
    for (;;)
    {
        //定期检查与执行lvgl所有已注册的周期性任务(诸如界面元素动画更新、定时数据刷新、系统状态监测等操作)
        lv_task_handler();

        //[若系统只有1个任务，可以不加，否则该任务无法运行]睡眠5个时钟节拍
//		xSemaphoreTake(xMutex,portMAX_DELAY);
        vTaskDelay(5);
//		xSemaphoreGive(xMutex);
    }
}

//超声波感应
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

//mq2监控
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
			
			//上报信息到平台服务器
			mqtt_publish_data(MQTT_PUBLISH_TOPIC,buf,0);
			
			//发送属于MQ2的事件
			xEventGroupSetBitsFromISR(xCreatedEventGroup,EVENT_GROUP_MQ2,NULL);
			
			last_mq2_status = mq2_status;
		}		
			vTaskDelay(50);
			
	}
}

//火焰传感器监控
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
			
			//上报信息到平台服务器
			mqtt_publish_data(MQTT_PUBLISH_TOPIC,buf,0);
			
			//发送属于FIRE的事件
			xEventGroupSetBitsFromISR(xCreatedEventGroup,EVENT_GROUP_FIRE,NULL);
			
			last_fire_status = fire_status;
		}		
			vTaskDelay(50);
		//printf("fire : %d\n\r",(int )FIRE);
			
	}

}

//异常处理任务
static void app_task_anomaly(void *pvParameters)
{
	EventBits_t eb;
    for (;;)
    {
		//接收到事件
		eb=xEventGroupWaitBits(xCreatedEventGroup,EVENT_GROUP_FIRE|EVENT_GROUP_MQ2,pdTRUE,pdFALSE,portMAX_DELAY);
		
		//火焰传感器事件
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

		//mq2事件
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
		//发送心跳包
		mqtt_send_heart();
		
		//上报设备状态
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
		
		/* n毫秒后，发现g_esp8266_rx_cnt没有变化，则认为接收数据结束 */
		if(g_esp8266_init && esp8266_rx_cnt && (esp8266_rx_cnt == g_esp8266_rx_cnt))
		{
			/* 发送消息，如果队列满了，超时时间为1000个节拍，如果1000个节拍都发送失败，函数直接返回 */
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
	
	//蜂鸣器嘀两声，D1灯闪烁两次，示意连接成功
	beep_on();  PFout(9)=0;delay_ms(100);
	beep_off(); PFout(9)=1;delay_ms(100);	
	beep_on();  PFout(9)=0;delay_ms(100);
	beep_off(); PFout(9)=1;delay_ms(100);

	printf("esp8266 connect aliyun with mqtt success\r\n");	
	
	vTaskResume(g_app_task_mqtt_handle);
	
	g_esp8266_init=1;
	
	for(;;)
	{	
		xReturn = xQueueReceive(g_queue_esp8266,	/* 消息队列的句柄 */
								buf,				/* 得到的消息内容 */
								portMAX_DELAY); 	/* 等待时间一直等 */
		if (xReturn != pdPASS)
		{
			dgb_printf_safe("[app_task_esp8266] xQueueReceive error code is %d\r\n", xReturn);
			continue;
		}	

		for(i=0;i<sizeof(buf);i++)
		{
			//判断的关键字符是否为 1"
			//核心数据，即{"switch_led_1":1}中的“1”
			if((buf[i]==0x31) && (buf[i+1]==0x22))
			{
					//判断控制变量
					if( buf[i+3]=='1' )
						PFout(9)=0;//控制灯亮
					else
						PFout(9)=1;//控制灯灭
			}	

			//判断的关键字符是否为 2"
			//核心数据，即{"switch_led_2":1}中的“1”
			if((buf[i]==0x32) && (buf[i+1]==0x22))
			{
					//判断控制变量
					if( buf[i+3]=='1' )
						PFout(10)=0;//控制灯亮
					else
						PFout(10)=1;//控制灯灭
			}

			//判断的关键字符是否为 3"
			//核心数据，即{"switch_led_3":1}中的“1”
			if(buf[i]==0x33 && buf[i+1]==0x22)
			{
					//判断控制变量
					if( buf[i+3]=='1' )
						PEout(13)=0;//控制灯亮
					else
						PEout(13)=1;//控制灯灭
			}				
		}

	}
}

/**
 * @brief 软件定时器任务
 * @param pvParameters:创建任务时传递的参数
 * @retval 无
 * @details 倒数管理、独立看门狗喂狗
 */
static void soft_timer_callback(TimerHandle_t pxTimer)
{
static 
	uint32_t count_backwards_sta=0;
//	oled_t oled;
//	BaseType_t xReturn;
	uint8_t buf[16] = {0};
	int32_t timer_id = (int32_t)pvTimerGetTimerID(pxTimer);

	/* 倒数状态成立 且 统计系统无操作计数值为0 或 g_system_no_opreation_cnt达到熄屏时间值 */
	if (count_backwards_sta && (g_system_no_opreation_cnt == 0 || g_system_no_opreation_cnt >= SCREEN_OFF_TIME))
	{
		/* 清空记录倒数状态 */
		count_backwards_sta = 0;


		memset(buf, 0, sizeof buf);
		sprintf((char *)buf, "%d", SCREEN_OFF_TIME - g_system_no_opreation_cnt);


	}

	/* 开始倒数,3 2 1 */
	if ((g_system_no_opreation_cnt >= SCREEN_OFF_TIME - 3) && (g_system_no_opreation_cnt < SCREEN_OFF_TIME))
	{
		dgb_printf_safe("[soft_timer_callback id=%d] g_system_no_opreation_cnt is %d\r\n", timer_id, g_system_no_opreation_cnt);

		/* 记录倒数状态 */
		count_backwards_sta=1;

		memset(buf, 0, sizeof buf);
		sprintf((char *)buf, "%d", SCREEN_OFF_TIME - g_system_no_opreation_cnt);

	}

	/* g_system_no_opreation_cnt达到熄屏时间值，则执行OLED熄屏*/
	if (g_system_no_opreation_cnt >= SCREEN_OFF_TIME)
	{
		
	}

	/* 进入临界区，FreeRTOS同时会关闭中断（UCOS的OS_CRITICAL_ENTER则停止任务调度） */
	taskENTER_CRITICAL();

#if IWDG_ENABLE
	/* 喂狗，刷新自身计数值 */
	IWDG_ReloadCounter();
#endif
	/* 统计系统无操作计数值自加1 */
	g_system_no_opreation_cnt++;

	/* 退出临界区，FreeRTOS同时会打开中断（UCOS的OS_CRITICAL_EXTI则恢复任务调度） */
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
