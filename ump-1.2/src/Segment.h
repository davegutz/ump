// Image segmentation bookkeeping functions
// DA Gutz 17-Jun-01
/* 	$Id: Segment.h,v 1.2 2007/02/09 23:47:52 davegutz Exp davegutz $	 */

#ifndef Segment_h
#define Segment_h 1

// Includes section
#include <stdlib.h>
#include <sys/param.h> // built-in constants and MIN/MAX
#include <curve.h>     // local curve lookup class
#include <limits.h>    // INT_MAX etc
#include <zap.h>       // local
#include "PixelLoc.h"  // local

// Globals
extern int verbose;    // print lots of information
extern int calibrate;  // calibrating
class Image;
typedef int (*cmp_func)(const void *, const void *);  // keep compiler happy

// Sort structure for Eigenvector
struct location{
  float x;                         // x value
  int   y;                         // y value, index
};

// Sort comparison for Eigenvector
typedef int (*cmp_func)(const void *, const void *);  // keep compiler happy
int comparelocs(const location *left, const location *right);

// Ball selection criteria structure
struct ballTune{
  float minBallDensity;
  float minBallRatio;
  float maxBallRatio;
  float minBallArea;
  float maxBallArea;
  float minProx;
};


// Approximation to exponential over expected range in function 'gaussian'
// Use equally spaced table lookup for throughput savings over exp function.
const float EXPOMIN = -5.;         // Limit
const float EXPOMAX =  0.;         // Limit
const int   EXPON   =  51;         // Number of breakpoints
static float EXPOZ[EXPON]={
 6.737947e-03, 7.446583e-03, 8.229747e-03, 9.095277e-03,
 1.005184e-02, 1.110900e-02, 1.227734e-02, 1.356856e-02, 1.499558e-02,
 1.657268e-02, 1.831564e-02, 2.024191e-02, 2.237077e-02, 2.472353e-02,
 2.732372e-02, 3.019738e-02, 3.337327e-02, 3.688317e-02, 4.076220e-02,
 4.504920e-02, 4.978707e-02, 5.502322e-02, 6.081006e-02, 6.720551e-02,
 7.427358e-02, 8.208500e-02, 9.071795e-02, 1.002588e-01, 1.108032e-01,
 1.224564e-01, 1.353353e-01, 1.495686e-01, 1.652989e-01, 1.826835e-01,
 2.018965e-01, 2.231302e-01, 2.465970e-01, 2.725318e-01, 3.011942e-01,
 3.328711e-01, 3.678794e-01, 4.065697e-01, 4.493290e-01, 4.965853e-01,
 5.488116e-01, 6.065307e-01, 6.703200e-01, 7.408182e-01, 8.187308e-01,
 9.048374e-01, 1.000000e+00
};
static Curve expo(EXPOMIN, EXPOMAX, EXPOZ, EXPON);

/////////////////////////////class Pixel///////////////////////////////
// Individual pixel class
class Pixel
{
private:
  int file_image_;
public:
  Pixel()
    : file_image_(-1), velp(NULL), valp(NULL), ratep(NULL),
    valpastp(NULL), x(0), y(0), onEdge(0), inMask(0), onCorner(0)
    {};

  // memImage Constructor
  Pixel(int *gray_velp, unsigned char *gray_valp)
    : file_image_(0), velp(gray_velp), valp(gray_valp), ratep(NULL),
    valpastp(NULL), x(0), y(0), onEdge(0), inMask(0), onCorner(0)
    {};

  // fileImage Constructor
  Pixel(unsigned char *gray_valp, int *gray_ratep,
	unsigned char *gray_valpastp, const int i, const int j)
    : file_image_(1), velp(NULL), valp(gray_valp), ratep(gray_ratep),
      valpastp(gray_valpastp), x(i), y(j), onEdge(0), inMask(0), onCorner(0)
    {};

  // Copy constructor
  Pixel(const Pixel & pix)
    : file_image_(pix.file_image_), velp(pix.velp), valp(pix.valp),
    ratep(pix.ratep), valpastp(pix.valpastp), x(pix.x), y(pix.y),
    onEdge(pix.onEdge), inMask(pix.inMask), onCorner(pix.onCorner)
    {};

  // Assignment operator
  Pixel & operator=(const Pixel & pix)
    {
      if ( this == &pix ) return *this;  // Allows self-assignment.
      file_image_ = pix.file_image_;
      velp        = pix.velp;
      valp        = pix.valp;
      ratep       = pix.ratep;
      valpastp    = pix.valpastp;
      x           = pix.x;
      y           = pix.y;
      onEdge      = pix.onEdge;
      inMask      = pix.inMask;
      onCorner    = pix.onCorner;
      return *this;  // Allows stacked assignments.
    }

  // Destructor
  ~Pixel(){}
  int           *velp;       // Gray scale pixel rate value
  unsigned char *valp;       // Gray scale pixel value
  int           *ratep;      // Gray scale pixel rate value
  unsigned char *valpastp;   // Gray scale past pixel value
  int            x;          // Spot from left of base image
  int            y;          // Spot from top of base image
  int            onEdge;     // Pixel is on edge of mask or image
  int            inMask;     // Pixel is in mask
  int            onCorner;   // Pixel is an intentionally added corner
  friend float gaussian(const Pixel & p1, const Pixel & p2,
			const float agauss, const float bgauss);
  // Element access
  int value();
  int velrate();
  friend ostream & operator<< (ostream & out, const Pixel &P);
};

// For sorts
int comparePixel(const Pixel *left, const Pixel *right);
int comparePixelPtr(const Pixel* *left, const Pixel* *right);

/////////////////////////////class Hypotenuse//////////////////////////
// Calculate distance between points with table lookup to save throughput
class Hypotenuse
{
private:
  int size_;
  float **value_;
public:
  Hypotenuse() : size_(0), value_(NULL){};
  Hypotenuse(const int size);
  Hypotenuse(const Hypotenuse &HY);
  Hypotenuse & operator=(const Hypotenuse & HY);
  ~Hypotenuse();
  float value(const int idelta, const int jdelta);
};

///////////////////////////class Point//////////////////////////////////
template <class T> class Point
{
protected:
  T x_;
  T y_;
public:
  Point() : x_(0), y_(0) {}
  Point(const Point <T> &P) : x_(P.x_), y_(P.y_) {}
  Point(const T x, const T y) : x_(x), y_(y){}
  ~Point(){}
  T x(){return x_;}
  T y(){return y_;}
  friend ostream & operator<< (ostream & out, const Point &P)
  {
    out << P.x_ << "," << P.y_;
    return out;
  }
};

///////////////////////////class Line//////////////////////////////////
template <class T> class Line
{
protected:
  float NaN;
  Point <T>   l_;
  Point <T>   r_;
  float       slope_;
public:
  Line(): NaN(1.e32), l_(Point<T>(T(0),T(0))), r_(Point<T>(T(0),T(0))),
						   slope_(NaN) {}
  Line(const Line<T> &L): NaN(1.e32), l_(L.l_), r_(L.r_), slope_(L.slope_) {}
  Line(const Point <T> l, const Point <T> r)
    : NaN(1.e32), l_(l), r_(r), slope_(NaN)
  {
    if ( (int) (r_.x() - l_.x()) )
      slope_ = (float) (r_.y() - l_.y()) / 
	       (float) (r_.x() - l_.x());
  }
  Line(const T lx, const T ly, const T rx, const T ry)
    : NaN(1.e32), l_(Point<T>(lx,ly)), r_(Point<T>(rx,ry)), slope_(NaN)
  {
    if ( (int) (r_.x() - l_.x()) )
      slope_ = (float) (r_.y() - l_.y()) / 
	       (float) (r_.x() - l_.x());
  }
  virtual ~Line(){}
  virtual float length()
    {
      float dx = l_.x() - r_.x();
      float dy = l_.y() - r_.y();
      return sqrt(dx*dx + dy*dy);
    }
  Point <T> l(){return l_;}
  Point <T> r(){return r_;}
  float slope(){return slope_;}
  float vertDist(const int i, const int j)
  {
    if ( slope_ < NaN )
      return ( (float)(i - l_.x()) * slope_ + l_.y() - (float)j );
    else
      return NaN;
  }    
  float vertDist(const float x, const float y)
  {
    if ( slope_ < NaN )
      return ( (x - (float)l_.x()) * slope_ + (float)l_.y() - y );
    else
      return NaN;
  }
  float *vertDistP(const float x, const float y)
  {
    float *vDistP;
    if ( slope_ < NaN
	 && ( ( x >= l_.x() && x <= r_.x() )
	      || ( x >= r_.x() && x <= l_.x() ) ) )
      {
	vDistP = new float( (x - (float)l_.x()) * slope_ + (float)l_.y() - y);
      }
    else
      {
	vDistP = NULL;
      }
    return vDistP;
  }
  float horizDist(const float x, const float y)
  {
    if ( 0 != slope_ )
      return ( (float)l_.x()  - x - ( (float)l_.y() -y ) / slope_ );
    else
      return NaN;
  }

  friend ostream & operator<< (ostream & out, const Line &L)
  {
    out << "left="   << L.l_     << " right=" << L.r_ 
	<< " slope=" << L.slope_ << endl;
    return out;
  }

};

class GridLine : public Line <int>
{
protected:
  Hypotenuse *distance_;
public:
  GridLine() : Line <int> (), distance_() {}
  GridLine(const GridLine &L)
    : Line <int> ( L.l_, L.r_ ), distance_(L.distance_) {}
  GridLine(const Point <int> l, const Point <int> r, Hypotenuse *distance)
    : Line <int> (l, r), distance_(distance) {}
  GridLine(const int lx, const int ly, const int rx, const int ry,
	   Hypotenuse *distance)
    : Line <int> (lx, ly, rx, ry), distance_(distance) {}
  GridLine & operator=(const GridLine & L)
    {
      if ( this == &L ) return *this;  // Allows self-assignment.
      l_  = L.l_;
      r_  = L.r_;
      distance_    = L.distance_;
      return *this;  // Allows stacked assignments.
    }
  ~GridLine(){}
  virtual float length()
    {
      return distance_->value(l_.x()-r_.x(), l_.y()-r_.y());
    }
};


///////////////////////////class TwoDimObject(TDO)///////////////////////////
struct pixelLoc{
  int i;
  int j;
};

class TDO
{
protected:
  int          count_;     // actual loaded number of pixels
  int          n_;         // reserved space for pixels
  Pixel      **picturePtr_;// pixels
  Hypotenuse  *distance_;  // pointer to class that calcs distance 
  GridLine   **line_;     // segments connecting pixels
  pixelLoc    *perim_;     // perimeter locations
  pixelLocf    centroid_;  // corrected centroid location
  int          nLine_;     // number lines connecting pixels
  int          nPer_;      // number points around perimeter
  float        value_;     // mean value of pixels
  int          imin_;      // left side
  int          imax_;      // right side
  int          jmin_;      // bottom side of enclosing rectangle
  int          jmax_;      // top side of enclosing rectangle
  int          area_;      // area enclosed by perimeter
  float        ratio_;     // perimeter area fraction of rectangle
  float        density_;   // fill ratio of pixels
  int          neutral_;   // defined neutral value
  Image       *image_;     // for access to correctRx()
  int          onEdge_;    // object is on edge
  int          onCorner_;  // object contains corner pixel
  int          loadTest_;  // performing load test
  pixelLocf    centroidDist_; // distorted centroid location
public:
  TDO()
    : count_(0), n_(0), picturePtr_(NULL), distance_(NULL), line_(NULL),
    perim_(NULL), centroid_(apixelLocf(0.0,0.0)),nLine_(0), nPer_(0), value_(0),
    imin_(0), imax_(0), jmin_(0), jmax_(0), area_(0), ratio_(0), density_(0),
    neutral_(0), image_(NULL), onEdge_(0), onCorner_(0), loadTest_(0),
    centroidDist_(apixelLocf(0.0,0.0)) {}
  TDO(const int nloc, Hypotenuse *distance, const int neutral, Image *image)
    : count_(0), n_(nloc), picturePtr_(NULL), distance_(distance), line_(NULL),
    perim_(NULL), centroid_(apixelLocf(0.0,0.0)), nLine_(0), nPer_(0),
    value_(0), imin_(0), imax_(0), jmin_(0), jmax_(0), area_(0), ratio_(0),
    density_(0), neutral_(neutral), image_(image), onEdge_(0), onCorner_(0),
    loadTest_(0), centroidDist_(apixelLocf(0.0,0.0)) 
  {
    picturePtr_ = new Pixel*[n_];
  }
  TDO(const TDO &O)
    : count_(O.count_), n_(O.n_),  picturePtr_(NULL), distance_(O.distance_), 
    line_(NULL), perim_(NULL), centroid_(O.centroid_), nLine_(O.nLine_),
    nPer_(O.nPer_), value_(O.value_), imin_(O.imin_), imax_(O.imax_),
    jmin_(O.jmin_), jmax_(O.jmax_), area_(O.area_), ratio_(O.ratio_),
    density_(O.density_), neutral_(O.neutral_), image_(O.image_),
    onEdge_(O.onEdge_), onCorner_(O.onCorner_), loadTest_(O.loadTest_),
    centroidDist_(O.centroidDist_)
  {
    picturePtr_ = new Pixel*[n_];
    line_       = new GridLine *[nLine_];
    perim_      = new pixelLoc[nPer_*2];
    for (int j=0; j<count_; j++)
      picturePtr_[j] = O.picturePtr_[j];
    for (int j=0; j<nLine_; j++)
      line_[j] = new GridLine (*(O.line_[j]));
    for (int j=0; j<nPer_*2; j++)
      perim_[j] = O.perim_[j];
  }
  TDO(TDO *Op)
    : count_(Op->count_), n_(Op->n_), picturePtr_(NULL),
    distance_(Op->distance_), line_(NULL), perim_(NULL),
    centroid_(Op->centroid_), nLine_(Op->nLine_), nPer_(Op->nPer_),
    value_(Op->value_), imin_(Op->imin_), imax_(Op->imax_), jmin_(Op->jmin_),
    jmax_(Op->jmax_), area_(Op->area_), ratio_(Op->ratio_),
    density_(Op->density_), neutral_(Op->neutral_), image_(Op->image_),
    onEdge_(Op->onEdge_), onCorner_(Op->onCorner_), loadTest_(Op->loadTest_),
    centroidDist_(Op->centroidDist_)
  {
    picturePtr_ = new Pixel*[n_];
    line_       = new GridLine *[nLine_];
    perim_      = new pixelLoc[nPer_*2];
    for (int j=0; j<count_; j++)
      picturePtr_[j] = Op->picturePtr_[j];
    for (int j=0; j<nLine_; j++)
      line_[j] = new GridLine (*(Op->line_[j]));
    for (int j=0; j<nPer_*2; j++)
      perim_[j] = Op->perim_[j];
  }

  TDO & operator=(const TDO & O); // Assignment
  virtual ~TDO()
  {
    deleteLines();
  }

  // Element access
  int       count()           {return count_;}
  int       area()            {return area_;}
  int       imax()            {return imax_;}
  int       imin()            {return imin_;}
  int       jmax()            {return jmax_;}
  int       jmin()            {return jmin_;}
  int       neutral()         {return neutral_;}
  float     density()         {return density_;}
  float     value()           {return value_;}
  int       onEdge()          {return onEdge_;}
  int       onCorner()        {return onCorner_;}
  int       loadTest()        {return loadTest_;}
  pixelLocf centroid()        {return centroid_;}
  Pixel    *picturePtr(int i) {return picturePtr_[i];}
  pixelLocf centroidDist()    {return centroidDist_;}

  // Functions
  void reset();              // Reset counters if re-using class without delete
  void deleteLines();        // Delete line & perimeter information
  int loadObjPixel(Pixel *pixelPtr); // Load points
  void showLines();          // Print lines for debug
  void calcProps();          // Calculate properties for a segment
  int absorbOther (const TDO & T);   // Consolidate
  pixelLoc bottom(int x, GridLine **line); // Find bottom of perimeter
  pixelLoc top(int x, GridLine **line);    // Find top of perimeter
  void sort();               // Sort points
  int isWhiter();            // Calculate intensity white
  int isBall(const ballTune T, const int verbose);  // Check for ball shape

  // Operators
  friend ostream & operator<< (ostream & out, const TDO &O);
};

// For sorts
int compareTDOptr(TDO* *left, TDO* *right);

///////////////////////////class SegmentSet/////////////////////////////
// SegmentSet class
class SegmentSet
{
protected:
  int             m_;                   // Parent image width
  int             n_;                   // Parent image height
  int             size_;                // Max allowed number of segments
  int             nS_;                  // Number of segments used
  int             sub_;                 // Number of subsampling
  TDO           **s_;                   // Array of segments
  Hypotenuse     *distance_;            // Pointer to distance calculator
public:
  SegmentSet(): m_(0), n_(0), size_(0), nS_(0), sub_(0), s_(NULL),
    distance_(NULL) {}
  SegmentSet(int s, Hypotenuse *D, int m, int n, int sub, Image *image):
    m_(m), n_(n), size_(s), nS_(0), sub_(sub), s_(NULL), distance_(D)
  {
    s_   = new TDO*[size_];
    for (int i=0; i<size_; i++) s_[i] = new TDO(size_, distance_, 0, image);
  }
  SegmentSet(const SegmentSet & S) : m_(S.m_), n_(S.n_), size_(S.size_),
    nS_(S.nS_), sub_(S.sub_), s_(NULL), distance_(NULL)
    {
      s_   = new TDO*[size_];
      for (int i=0; i<size_; i++) s_[i] = new TDO(S.s_[i]);
    }
  SegmentSet & operator=(const SegmentSet & S)
    {
      if ( this == &S ) return *this;  // Allows self-assignment.
      m_    = S.m_;
      n_    = S.n_;
      size_ = S.size_;
      nS_   = S.nS_;
      sub_  = S.sub_;
      s_   = new TDO*[size_];
      for (int i=0; i<size_; i++) s_[i] = new TDO(S.s_[i]);
      distance_ = NULL;
      return *this;                    // Allows stacked assignments.
    }
  ~SegmentSet()
    {
      for (int i=0; i<size_; i++) if ( s_[i] ) zap(s_[i]);
      if ( s_       ) zaparr(s_);
    }
  
  // Access to values
  inline int segLoc(int j, int i)
  {
      return n_*((s_[j]->picturePtr(i))->y) + (s_[j]->picturePtr(i))->x;
  }  
  inline int  segLocX   (int j, int i)
    {
      return (s_[j]->picturePtr(i))->x;
    }  
  inline int  segLocY   (int j, int i)
    {
      return (s_[j]->picturePtr(i))->y;
    }
  inline int  segLocSub (int j, int i)
  {
      return
	n_/sub_ * int ( float((s_[j]->picturePtr(i))->y) / float(sub_)) +
	          int ( float((s_[j]->picturePtr(i))->x) / float(sub_));
  }  
  inline int  segLocSubX(int j, int i)
    {
      return int ( float((s_[j]->picturePtr(i))->x) / float(sub_));
    }  
  inline int  segLocSubY(int j, int i)
    {
      return int ( float((s_[j]->picturePtr(i))->y) / float(sub_));
    }
  inline int  nSeg()              
    {
      return nS_;
    }
  inline int  nSegLoc(int j)     
    {
      return s_[j]->count();
    }
  inline TDO *segP(int j) 
    {
      return s_[j];
    }

  // Functions
  void resetNodes();              // Reset storage each pass
  void loadNode(int j, Pixel *p); // Load a new node into separate segments
  void calcProperties();          // Calculate properties for segments
  void sort();                    // Sort
  void consolidate(ballTune C);   // Consolidate close object that would not
                                  // segregate due to clean image.

  // Operators
  friend ostream & operator<< (ostream & out, const SegmentSet &SS);
};

#endif // Segment_h

