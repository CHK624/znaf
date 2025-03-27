#ifndef __LED_H__
#define __LED_H__

#include <sys.h>

#define D1 PFout(9)
#define D2 PFout(10)
#define D3 PEout(13)
#define D4 PEout(14)

extern void led_init(void);



#endif
