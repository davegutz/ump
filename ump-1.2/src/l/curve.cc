// Curve lookup utilities
// DA Gutz 17-Jun-01
// 	$Id: curve.cc,v 1.1 2007/02/10 00:57:44 davegutz Exp davegutz $	

// Includes section
using namespace std;
#include <common.h>
#include <math.h>
#include "curve.h"


// binsearch
// Find x in { v[0] <= v[1] <= ... ,= v[n-1] } and calculate 
// the fraction of range that x is positioned.
// Inputs:
// x     Input to vector.
// n     Size of vector.
// v     Vector.
// Outputs:
// *dx   Fraction of range for x.
// *low  Current low end of range.
// *high Current high end of range.

void binsearch(float x, float *v, int n, int *high, int *low, 
                float *dx)
{
    int mid;

    /* Initialize high and low  */
    *low    = 0;
    *high   = n-1;

    /* Check endpoints  */
    if ( x >= *(v+*high) )
      {
        *low    = *high;
        *dx     = 0.;
      }
    else if ( x <= *(v+*low) )
      {
        *high   = *low;
        *dx     = 0.;
    }

    /* Search if necessary  */
    else
      {
        while( (*high -  *low) > 1 )
	  {
            mid = (*low + *high) / 2;
            if ( *(v+mid) > x )
	      *high   = mid;
            else
	      *low    = mid;
	  }
        *dx = (x - *(v+*low)) / (*(v+*high) - *(v+*low));
      }
}   /* End binsearch    */

float curvex(float x, float y, Curve2 & curve)
{
  float dx,    dy,   r0,    r1;
  int   xhigh, xlow, yhigh, ylow;

  binsearch(x, curve.breakpoints_,  curve.nbrk_, &xhigh, &xlow, &dx);
  binsearch(y, curve.breakpointsY_, curve.mbrk_, &yhigh, &ylow, &dy);

  r0 = curve.table_[ylow*curve.nbrk_ + xlow]
    + dx * ( curve.table_[ylow*curve.nbrk_ + xhigh]
	     - curve.table_[ylow*curve.nbrk_ + xlow] );

  r1 = curve.table_[yhigh*curve.nbrk_ + xlow]
    + dx * ( curve.table_[yhigh*curve.nbrk_ + xhigh]
	     - curve.table_[yhigh*curve.nbrk_ + xlow] );

  return r0 + dy*(r1 - r0);
}


float curvex(float x, Curve & curve)
{
  float dx;
  int high, low;
  // Eq spaced version
  if ( curve.dx_ != 0 )
    {
      if ( x <= curve.xmin_ ) return curve.table_[0];
      else if ( x >= curve.xmax_ ) return curve.table_[curve.nbrk_-1];
      float frac = (x - curve.xmin_) / curve.dx_;
      low = (int)floor(frac);
      high = low + 1;
      dx = frac - low;
    }
  // Searched version
  else
    {
      if ( curve.nbrk_ <1 ) return curve.table_[0];
      binsearch(x, curve.breakpoints_, curve.nbrk_, &high, &low, &dx);
    }
  return   *(curve.table_+low)
    + dx * ( *(curve.table_+high) -  *(curve.table_+low) );
}

