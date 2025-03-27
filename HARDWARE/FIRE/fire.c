#include <stm32f4xx.h>
#include <fire.h>

void fire_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//1.����GPIOFʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);
	
	//2.��ʼ��GPIO
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;//����ģʽ
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;//��������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;//PG9
	GPIO_Init(GPIOG,&GPIO_InitStruct);
}
