#include <stm32f4xx.h>
#include <includes.h>


void sr04_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//1.����GPIOFʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOA,ENABLE);
	
	//2.��ʼ��GPIO
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;//����ģʽ
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;//��������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;//PA8
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;//���ģʽ
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;//�������--����
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;//����ٶ�
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;//��������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;//PE6
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	
	TRIG = 0;//Ĭ�������
}

//���,�ɹ����ؾ���(cm),ʧ�ܷ���-1
int get_distance(void)
{
	u32 retry = 0;
	
	//TRIG����>10us�ߵ�ƽ�ź�
	TRIG = 1;
	delay_us(15);
	TRIG = 0;
	
	//�ȴ�ECHO���,����60ms��û�б�ߣ�ֱ�ӷ���-1
	while(ECHO==0){
		retry++;
		delay_us(1);
		if(retry>60000)
			return -1;
	}
	
	retry = 0;
	//����ߵ�ƽʱ�䣬�ȴ�ECHO���
	while(ECHO){
		retry++;
		delay_us(10);
		if(retry>6000)
			return -1;
	}
	
	//�����
	return 10*retry/58;
}
