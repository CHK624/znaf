#include "includes.h"

DCMI_InitTypeDef DCMI_InitStructure;

// DCMI中断服务函数
void DCMI_IRQHandler(void)
{
	uint32_t i = 0;
	uint8_t *p = g_ov2640_row_rgb;
	uint8_t tmp;

	if (DCMI_GetITStatus(DCMI_IT_LINE) == SET) // 捕获到一行
	{

		//交换高低字节
		for (i = 0; i < 480 * 2; i += 2)
		{
			tmp = p[i];
			p[i] = p[i + 1];
			p[i + 1] = tmp;
		}

		//该标志位一定要清空，否则下一次传输会失败
		SPI_CS_0;
		DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_TCIF3);
		SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
		DMA_Cmd(DMA2_Stream3, ENABLE);
		PFout(9) ^= 1;
		DCMI_ClearITPendingBit(DCMI_IT_LINE); // 清除帧中断
	}
}
// DCMI DMA配置
// DMA_Memory0BaseAddr:存储器地址    将要存储摄像头数据的内存地址(也可以是外设地址)
// DMA_BufferSize:存储器长度    0~65535
// DMA_MemoryDataSize:存储器位宽    @defgroup DMA_memory_data_size :DMA_MemoryDataSize_Byte/DMA_MemoryDataSize_HalfWord/DMA_MemoryDataSize_Word
// DMA_MemoryInc:存储器增长方式  @defgroup DMA_memory_incremented_mode  /** @defgroup DMA_memory_incremented_mode : DMA_MemoryInc_Enable/DMA_MemoryInc_Disable

void dcmi_dma_init(u32 DMA_Memory0BaseAddr, u16 DMA_BufferSize, u32 DMA_MemoryDataSize, u32 DMA_MemoryInc)
{
	DMA_InitTypeDef DMA_InitStructure;


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); // DMA2时钟使能

	/* 配置 DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_1;							// 通道1 DCMI通道
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&DCMI->DR;				// 外设地址为:DCMI->DR
	DMA_InitStructure.DMA_Memory0BaseAddr = DMA_Memory0BaseAddr;			// DMA 存储器0地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;					// 外设到存储器模式
	DMA_InitStructure.DMA_BufferSize = DMA_BufferSize;						// 数据传输量
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc;						// 存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // 外设数据长度:32位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize;				// 存储器数据长度
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							// 使用循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						// 高优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;					// FIFO模式
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;			// 使用全FIFO
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				// 外设突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		// 存储器突发单次传输
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);								// 初始化DMA Stream
}

// DCMI初始化
void dcmi_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE, ENABLE); // 使能GPIOA B C E 时钟
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, ENABLE);
	// GPIOF9,F10初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6; // PA4/6   复用功能输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		   // 复用功能输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		   // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	   // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		   // 上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);				   // 初始化

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6; // PB6/7   复用功能输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);				   // 初始化

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_11; // PC6/7/8/9/11 复用功能输出
	GPIO_Init(GPIOC, &GPIO_InitStructure);														   // 初始化

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6; // PE5/6  复用功能输出
	GPIO_Init(GPIOE, &GPIO_InitStructure);				   // 初始化

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_DCMI);	 // PA4,AF13  DCMI_HSYNC
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_DCMI);	 // PA6,AF13  DCMI_PCLK
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_DCMI);	 // PB7,AF13  DCMI_VSYNC
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_DCMI);	 // PC6,AF13  DCMI_D0
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_DCMI);	 // PC7,AF13  DCMI_D1
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_DCMI);	 // PC8,AF13  DCMI_D2
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_DCMI);	 // PC9,AF13  DCMI_D3
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_DCMI); // PC11,AF13 DCMI_D4
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_DCMI);	 // PB6,AF13  DCMI_D5
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_DCMI);	 // PE5,AF13  DCMI_D6
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF_DCMI);	 // PE6,AF13  DCMI_D7

	DCMI_DeInit(); // 清除原来的设置

	DCMI_InitStructure.DCMI_CaptureMode = DCMI_CaptureMode_Continuous;	 // 连续模式
	DCMI_InitStructure.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;	 // 全帧捕获
	DCMI_InitStructure.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b; // 8位数据格式
	DCMI_InitStructure.DCMI_HSPolarity = DCMI_HSPolarity_Low;			 // HSYNC 低电平有效
	DCMI_InitStructure.DCMI_PCKPolarity = DCMI_PCKPolarity_Rising;		 // PCLK 上升沿有效
	DCMI_InitStructure.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;	 // 硬件同步HSYNC,VSYNC
	DCMI_InitStructure.DCMI_VSPolarity = DCMI_VSPolarity_Low;			 // VSYNC 低电平有效
	DCMI_Init(&DCMI_InitStructure);

	DCMI_ITConfig(DCMI_IT_LINE, ENABLE); // 开启行结束中断

	DCMI_Cmd(ENABLE); // DCMI使能

	NVIC_InitStructure.NVIC_IRQChannel = DCMI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		  // 子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器
}
// DCMI,启动传输
void dcmi_start(void)
{
	DCMI_Cmd(ENABLE);			   // DCMI使能
	DMA_Cmd(DMA2_Stream1, ENABLE); // 开启DMA2,Stream1
	DCMI_CaptureCmd(ENABLE);	   // DCMI捕获使能
}
// DCMI,关闭传输
void dcmi_stop(void)
{
	DCMI_CaptureCmd(DISABLE); // DCMI捕获使关闭

	while (DCMI->CR & 0X01)
		; // 等待传输结束

	DMA_Cmd(DMA2_Stream1, DISABLE); // 关闭DMA2,Stream1

	DCMI_Cmd(DISABLE); // DCMI关闭
}

