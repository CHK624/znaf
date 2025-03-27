#ifndef _SR04_H_
#define _SR04_H_

#include <sys.h>

#define ECHO PAin(8)
#define TRIG PEout(6)

void sr04_init(void);
int get_distance(void);

#endif
