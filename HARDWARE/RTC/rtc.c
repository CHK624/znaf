#include "includes.h"


volatile uint32_t g_rtc_wakeup_event=0;

void rtc_init(void)
{

		
	/* Enable the PWR clock��ʹ�ܵ�Դ����ʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	
	/* Allow access to RTC���������RTC */
	PWR_BackupAccessCmd(ENABLE);

#if RTC_CLK_SRC_SEL == RTC_CLK_SRC_IS_LSE
	//��LSE��ʱ��
	RCC_LSEConfig(RCC_LSE_ON);

	//���LSE��ʱ���Ƿ����
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
	
	//ΪRTCѡ��LSE��Ϊʱ��Դ
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
#endif

#if RTC_CLK_SRC_SEL == RTC_CLK_SRC_IS_LSI	
	RCC_LSICmd(ENABLE);
	
	/* Wait till LSI is ready */  
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{
	}
	
	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#endif
	
	/* Enable the RTC Clock��ʹ��RTC��Ӳ��ʱ�� */
	RCC_RTCCLKCmd(ENABLE);
	
	/* Wait for RTC APB registers synchronisation���ȴ�����RTC��ؼĴ������� */
	RTC_WaitForSynchro();
	
#if RTC_CLK_SRC_SEL == RTC_CLK_SRC_IS_LSE
	/* Configure the RTC data register and RTC prescaler������RTC���ݼĴ�����RTC�ķ�Ƶֵ */
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;				//�첽��Ƶϵ��
	RTC_InitStructure.RTC_SynchPrediv = 0xFF;				//ͬ����Ƶϵ��
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	//24Сʱ��ʽ
	RTC_Init(&RTC_InitStructure);
#endif

#if RTC_CLK_SRC_SEL == RTC_CLK_SRC_IS_LSI	
	/* Configure the RTC data register and RTC prescaler������RTC���ݼĴ�����RTC�ķ�Ƶֵ */
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;				//�첽��Ƶϵ��
	RTC_InitStructure.RTC_SynchPrediv = 0xF9;				//ͬ����Ƶϵ��
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	//24Сʱ��ʽ
	RTC_Init(&RTC_InitStructure);

#endif
	
	//��������
	RTC_DateStructure.RTC_Year = 24;				//2024��
	RTC_DateStructure.RTC_Month = RTC_Month_March;	//3�·�
	RTC_DateStructure.RTC_Date = 26;				//��26��
	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Tuesday;//���ڶ�
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);


	//����ʱ�� 
	RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
	RTC_TimeStructure.RTC_Hours   = 10;
	RTC_TimeStructure.RTC_Minutes = 30;
	RTC_TimeStructure.RTC_Seconds = 00; 
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);  
	
	RTC_WakeUpCmd(DISABLE);
	
	//����ʱ��Դ��Ӳ��ʱ��Ƶ��Ϊ1Hz
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	
	//ֻ����һ�μ���
	RTC_SetWakeUpCounter(0);
	
	RTC_WakeUpCmd(ENABLE);

	//�����жϵĴ�����ʽ�������ж�
	RTC_ITConfig(RTC_IT_WUT, ENABLE);
	
	RTC_ClearFlag(RTC_FLAG_WUTF);
	
	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//RTC�ֲ�涨
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	//���ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}


void  RTC_WKUP_IRQHandler(void)
{
	//����־λ
	if(RTC_GetITStatus(RTC_IT_WUT) == SET)
	{
		g_rtc_wakeup_event = 1;
		//��ձ�־λ	
		RTC_ClearITPendingBit(RTC_IT_WUT);
		
		EXTI_ClearITPendingBit(EXTI_Line22);
		
	}
	
}


