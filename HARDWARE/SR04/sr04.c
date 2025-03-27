#include <stm32f4xx.h>
#include <includes.h>


void sr04_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//1.开启GPIOF时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOA,ENABLE);
	
	//2.初始化GPIO
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;//输入模式
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;//PA8
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;//输出模式
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;//输出类型--推挽
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;//输出速度
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;//PE6
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	
	TRIG = 0;//默认输出低
}

//测距,成功返回距离(cm),失败返回-1
int get_distance(void)
{
	u32 retry = 0;
	
	//TRIG发送>10us高电平信号
	TRIG = 1;
	delay_us(15);
	TRIG = 0;
	
	//等待ECHO变高,超过60ms还没有变高，直接返回-1
	while(ECHO==0){
		retry++;
		delay_us(1);
		if(retry>60000)
			return -1;
	}
	
	retry = 0;
	//计算高电平时间，等待ECHO变低
	while(ECHO){
		retry++;
		delay_us(10);
		if(retry>6000)
			return -1;
	}
	
	//求距离
	return 10*retry/58;
}
