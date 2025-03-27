#include <stm32f4xx.h>
#include <fire.h>

void fire_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//1.开启GPIOF时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);
	
	//2.初始化GPIO
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;//输入模式
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;//PG9
	GPIO_Init(GPIOG,&GPIO_InitStruct);
}
