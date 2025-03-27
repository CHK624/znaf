#ifndef _DCMI_H_
#define _DCMI_H_

#include "sys.h"


extern void dcmi_init(void);
extern void dcmi_dma_init(u32 DMA_Memory0BaseAddr, u16 DMA_BufferSize, u32 DMA_MemoryDataSize, u32 DMA_MemoryInc);
extern void dcmi_start(void);
extern void dcmi_stop(void);

#endif
