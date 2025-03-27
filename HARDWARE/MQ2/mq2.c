#include <stm32f4xx.h>
#include <mq2.h>

void mq2_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//1.开启GPIOF时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	
	//2.初始化GPIO
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;//输入模式
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;//PA6
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}
