#ifndef __RTC_H__
#define __RTC_H__

#define RTC_CLK_SRC_IS_LSE  0
#define RTC_CLK_SRC_IS_LSI  1

#define RTC_CLK_SRC_SEL     RTC_CLK_SRC_IS_LSI

extern void rtc_init(void);

#endif

