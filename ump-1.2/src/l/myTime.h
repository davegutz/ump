// Local timeval calculations
// DA Gutz 11-Feb-07
/* 	$Id: myTime.h,v 1.1 2007/02/11 14:13:14 davegutz Exp davegutz $	 */
#ifndef myTime_h
#define myTime_h 1

#include <sys/time.h>  // timing

timeval aClearTimeval();
float   timerdiff(const timeval high, const timeval low);
float   operator-(const timeval high, const timeval low);
void    timerassign(const float time, timeval *target);

#endif // myTime_h
