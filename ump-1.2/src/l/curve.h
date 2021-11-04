// Curve lookup utilities
// DA Gutz 17-Jun-01
/* 	$Id: curve.h,v 1.2 2007/02/11 03:09:49 davegutz Exp davegutz $	 */

#ifndef curve_h
# define curve_h 1


void binsearch(float x, float *v, int n, int *high, int *low, 
	       float *dx);

///////////////////////////class Curve/////////////////////////////////////
class Curve
{
 protected:
  float *breakpoints_;  // Breakpoint table.
  float *table_;        // Table data.
  int    nbrk_;         // Number of points.
  float  dx_;           // Equally spaced delta.
  float  xmin_;         // Min breakpoint for equally space table.
  float  xmax_;         // Max breakpoint for equally space table.
 public:
  Curve() : breakpoints_(NULL), table_(NULL), nbrk_(0), dx_(0), xmin_(0),
    xmax_(0) {};
  Curve(float *brk, float  *tab, const int n)
    : breakpoints_(brk), table_(tab), nbrk_(n), dx_(0), xmin_(0), xmax_(0) {}
  Curve(const float xmin, const float xmax, float  *tab, const int n)
    : breakpoints_(NULL), table_(tab), nbrk_(n), dx_((xmax-xmin)/(n-1)),
    xmin_(xmin), xmax_(xmax) {}
  Curve(const Curve & C)
    : breakpoints_(C.breakpoints_), table_(C.table_), nbrk_(C.nbrk_),
    dx_(C.dx_), xmin_(C.xmin_), xmax_(C.xmax_) {}
  Curve & operator=(const Curve & C)
    {
      if ( this == &C ) return *this;  // Allows self-assignment.
      breakpoints_ = C.breakpoints_;
      table_       = C.table_;
      nbrk_        = C.nbrk_;
      dx_          = C.dx_;
      xmin_        = C.xmin_; 
      xmax_        = C.xmax_;
      return *this;                    // Allows stacked assignments.
    }
  virtual ~Curve(){}
  // The lookup functions.
  friend float curvex(float x, Curve & curve);
};

///////////////////////////class Curve2/////////////////////////////////////
class Curve2 : public Curve
{
private:
  float *breakpointsY_;  // Breakpoint table.
  int    mbrk_;         // Number of points.
  float  dy_;           // Equally spaced delta.
public:
  Curve2() : Curve(), breakpointsY_(NULL), mbrk_(0), dy_(0) {}
  Curve2(float *brk, float *ybrk, float  *tab, const int n, const int m)
    : Curve(brk, tab, n), breakpointsY_(ybrk), mbrk_(m), dy_(0) {}
  Curve2(const Curve2 &C) : breakpointsY_(C.breakpointsY_),
    mbrk_(C.mbrk_), dy_(C.dy_) {}
  Curve2 & operator=(const Curve2 & C)
    {
      if ( this == &C ) return *this;  // Allows self-assignment.
      breakpointsY_ = C.breakpointsY_;
      mbrk_         = C.mbrk_;
      dy_           = C.dy_;
      return *this;                    // Allows stacked assignments.
    }
  ~Curve2(){}
  // The lookup functions.
  friend float curvex(float x, float y, Curve2 & curve);
};

#endif //curve_h





  

