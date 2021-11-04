// $Id: myTime.cc,v 1.1 2007/02/11 14:13:52 davegutz Exp davegutz $	
// Local timeval calculations
// DA Gutz 11-Feb-07

// Includes section
using namespace std;
#include <common.h>
#include "myTime.h"

// Initialize structure
timeval aClearTimeval()
{
  timeval temp;
  timerclear(&temp);
  return(temp);
}

// Timer calculations
float timerdiff(const timeval high, const timeval low)
{
  if ( !timerisset(&high) || !timerisset(&low) ) return 0.0;
  float  tmp =   (float)(high.tv_sec  - low.tv_sec)
               + (float)(high.tv_usec - low.tv_usec)/1000000.0;
  return tmp;
}
float operator-(const timeval high, const timeval low)
{
  return timerdiff(high, low);
}
void timerassign(const float time, timeval *target)
{
  timerclear(target);
  target->tv_sec  = (time_t) int(time);
  target->tv_usec = (suseconds_t)
    int( (time - (float)(target->tv_sec))*1000000.0 );
}
