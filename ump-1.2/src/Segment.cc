// Image segmentation bookkeeping functions
// DA Gutz 17-Jun-01
// 	$Id: Segment.cc,v 1.2 2007/02/10 00:03:40 davegutz Exp davegutz $ 

// Includes section
using namespace std;
#include <common.h>
#include <stdio.h>     // sprintf
#include <iostream>    // cerr etc
#include "Segment.h"   // local
#include "Image.h"     // local

// location functions
int comparelocs(const location *left, const location *right)
{
  return ( (left->x < right->x) ? -1 : (left->x == right->x) ? 0 : 1 );
}


// Calculate gaussian edge weight. Use pixel value to segment, not rate.
// Focus on object, not difference with background.
float gaussian(const Pixel & p1, const Pixel & p2,
	       const float agauss, const float bgauss)
{
  int   dx_, dy_;          // x and y coordinate distances.
  float dij_squared_;      // Square of normalized pixel space difference.
  float dIij_;             // Normalized pixel strength difference.
  float edgeweight_;       // Edge weighting.
  dx_          = p1.x - p2.x;
  dy_          = p1.y - p2.y;
  dij_squared_ = float(dx_*dx_ + dy_*dy_) / agauss / agauss;
  dIij_        = (*(p1.valp) - *(p2.valp)) / bgauss;
  //edgeweight_  = exp(-dij_squared_)          * exp(-(dIij_*dIij_));
  edgeweight_  = curvex(-dij_squared_, expo) * curvex(-dIij_*dIij_,  expo);
  return edgeweight_;
}
ostream & operator<< (ostream & out, const Pixel &P)
{
  if ( P.file_image_ )
    out << P.x << "," << P.y << " (" << *(P.ratep) << "/" 
	<< (int)*(P.valp) << ")";
  else 
    out <<  P.x << "," << P.y << " (" << (int)*(P.velp) << "/"
	<< (int)*(P.valp) << ")";
  return out;
}

// Element access
int Pixel::value()
{
  return (int)*valp;
}
int Pixel::velrate()
{
  if ( file_image_ ) return (*ratep);
  else               return (int) *velp;
}

Hypotenuse::Hypotenuse(const int size) : size_(size), value_(NULL)
{
  int n = size_*2+1;
  value_ = new float*[n];
  for (int i=0; i<n; i++) value_[i] = new float[n]; 
  for (int i=0; i<n; i++) for (int j=0; j<n; j++)
    value_[i][j] = sqrt( float((i-size_)*(i-size_) + (j-size_)*(j-size_)) );
}

Hypotenuse::Hypotenuse(const Hypotenuse & HY) : size_(HY.size_), value_(NULL)
{
  int n = size_*2+1;
  value_ = new float*[n];
  for (int i=0; i<n; i++) value_[i] = new float[n]; 
  for (int i=0; i<n; i++) for (int j=0; j<n; j++)
    value_[i][j] = HY.value_[i][j];
}

Hypotenuse & Hypotenuse::operator=(const Hypotenuse & HY)
{
  if ( this == &HY ) return *this;
  size_  = HY.size_;
  int n = size_*2+1;
  value_ = new float*[n];
  for (int i=0; i<n; i++) value_[i] = new float[n]; 
  for (int i=0; i<n; i++) for (int j=0; j<n; j++)
    value_[i][j] = HY.value_[i][j];
  return *this;   // Allow stacked assignments
}

Hypotenuse::~Hypotenuse()
{
  if ( value_ )
    {
      for (int i=0, n=size_*2+1; i<n; i++) if ( value_[i]) zaparr(value_[i]); 
      zaparr(value_);
    }
}
float Hypotenuse::value(const int idelta, const int jdelta)
{
  return value_[idelta+size_][jdelta+size_];
}



// Sorts
int comparePixel(const Pixel *left, const Pixel *right)
{
  if          ( left->x < right->x ) return -1;
  else if     ( left->x > right->x ) return  1;
  else
    {
      if      ( left->y < right->y ) return -1;
      else if ( left->y > right->y ) return  1;
      else return 0;
    }
}
int comparePixelPtr(const Pixel* *left, const Pixel* *right)
{
  if          ( (*left)->x < (*right)->x ) return -1;
  else if     ( (*left)->x > (*right)->x ) return  1;
  else
    {
      if      ( (*left)->y < (*right)->y ) return -1;
      else if ( (*left)->y > (*right)->y ) return  1;
      else return 0;
    }
}
int compareTDOptr(TDO* *left, TDO* *right)
{
  static const float minTDODensity =  0.1;
  if      ( (*left)->density()  < minTDODensity    ) return  1;
  else if ( (*right)->density() < minTDODensity    ) return -1;
  else if ( (*left)->area()     < (*right)->area() ) return  1;
  else if ( (*left)->area()     > (*right)->area() ) return -1;
  else
    {
      if      ( (*left)->value()   < (*right)->value() ) return  1;
      else if ( (*left)->value()   > (*right)->value() ) return -1;
      else return 0;
    }
}

/////////////////////////class TDO//////////////////////////////////////////
// Reset counters if re-using class without delete
void TDO::reset() {onEdge_ = onCorner_ = count_ = 0;}

// Delete line & perimeter information
void TDO::deleteLines()
{
  if ( picturePtr_ ) zaparr(picturePtr_);
  for (int j=0; j<nLine_; j++)
    {
      if ( line_[j] ) zap(line_[j]);
    }
  if ( nLine_  ) zaparr(line_);
  if ( nPer_   ) zaparr(perim_);
  nLine_    = 0;
  nPer_     = 0;
}

// Assignment
TDO & TDO::operator=(const TDO & O)
  {
    if(this==&O)return *this;  // Allows self-assignment.

    // Cleanup
    deleteLines();

    // Assign
    count_    = O.count_;
    n_        = O.n_;
    distance_ = O.distance_;
    centroid_ = O.centroid_;
    nPer_     = O.nPer_; 
    nLine_    = O.nLine_;
    value_    = O.value_;
    imin_     = O.imin_;
    imax_     = O.imax_;
    jmin_     = O.jmin_; 
    jmax_     = O.jmax_;
    area_     = O.area_;
    ratio_    = O.ratio_;
    density_  = O.density_;
    neutral_  = O.neutral_;
    image_    = O.image_;
    onEdge_   = O.onEdge_;
    onCorner_ = O.onCorner_;
    loadTest_ = O.loadTest_;
    centroidDist_ = O.centroidDist_;
    picturePtr_ = new Pixel*[n_];
    for (int j=0; j<count_; j++)
      {
	picturePtr_[j] = O.picturePtr_[j];
      }
    line_     = new GridLine *[nLine_];
    perim_    = new pixelLoc[nPer_*2];
    for (int j=0; j<nLine_;  j++) line_[j]  = new GridLine (*(O.line_[j]));
    for (int j=0; j<nPer_*2; j++) perim_[j] = O.perim_[j];
    return *this;  // Allows stacked assignments.
  }

// Load points
int TDO::loadObjPixel(Pixel *pixelPtr)
{
  picturePtr_[count_++] = pixelPtr;
  if ( count_ > n_ )
    {
      cerr << "Segment : " << __FILE__ << " : " << __LINE__ << " : "  
	   << " TDO over-filled in loadObjPixel\n";
      return (-1);
    }
  else return (0);
}

// Print lines for debug
void TDO::showLines()
{
  cerr << "nLine_=" << nLine_ << endl;
  for (int i=0; i<nLine_; i++)
    {
      cerr << "line=" << *line_[i];
    }
}

// Consolidate
int TDO::absorbOther (const TDO & T)
{
  if (this==&T) return (0);
  
  // Reconstruct picturePtr_
  n_ += T.n_;
  Pixel **tempPtr = new Pixel*[n_];
  for (int j=0; j<count_; j++) tempPtr[j] = picturePtr_[j];
  zaparr(picturePtr_);
  for (int j=count_; j<count_+T.count_; j++)
    tempPtr[j] = T.picturePtr_[j-count_];
  picturePtr_ = tempPtr;

  // Meld properties
  float total = (float)(count_ + T.count_);
  value_      = (value_         *(float)count_
		 + T.value_     *(float)T.count_) / total;
  centroid_.x = (centroid_.x    *(float)count_
		 + T.centroid_.x*(float)T.count_) / total;
  centroid_.y = (centroid_.y    *(float)count_
		 + T.centroid_.y*(float)T.count_) / total;
  centroidDist_.x = (centroidDist_.x    *(float)count_
		     + T.centroidDist_.x*(float)T.count_) / total;
  centroidDist_.y = (centroidDist_.y    *(float)count_
		     + T.centroidDist_.y*(float)T.count_) / total;
  area_      += T.area_;
  imin_       = MIN (imin_, T.imin_);
  imax_       = MAX (imax_, T.imax_);
  jmin_       = MIN (jmin_, T.jmin_);
  jmax_       = MAX (jmax_, T.jmax_);
  // NOTE:  this doesn't work if Inf value.  Hasn't been a real problem.
  ratio_      = (ratio_ + T.ratio_) / 2.0;
  density_    = total / (float) area_ * (float) image_->sub();
  count_     += T.count_;
  onEdge_     = MAX (onEdge_, T.onEdge_); 
  onCorner_   = MAX (onCorner_, T.onCorner_);
  if ( count_ > n_ )
    {
      cerr << "Segment : " << __FILE__ << " : " << __LINE__ << " : "  
	   << "TDO over-filled in absorbOther\n";
      return (-1);
    }
  else return (0);
}

// Find bottom of perimeter
pixelLoc TDO::bottom(int x, GridLine **line)
{
  pixelLoc retPix;
  retPix.i = x;
  int i = count_-1;
  while( picturePtr_[i]->x >= x )
    {
      if ( !i )
	{
	  retPix.j = picturePtr_[i]->y;
	  return retPix;
	}
      i--;
    }
  retPix.j = INT_MAX;
  for (int j=0; j<count_; j++) for (int i=j; i<count_; i++)
    {
      int index = j*count_+i;
      float *vDist = line[index]->vertDistP(x, 0);
      if ( vDist )
	{
	  retPix.j = MIN ( (int) rint(*vDist), retPix.j );
	  zap(vDist);
	}
    }
  // Average lines with a locally present pixel
  if ( picturePtr_[i+1]->x == x )
    retPix.j = (retPix.j + picturePtr_[i+1]->y) / 2;
  return retPix;
}
// Find top of perimeter
pixelLoc TDO::top(int x, GridLine **line)
{
  pixelLoc retPix;
  retPix.i = x;
  int i = 0;
  while( picturePtr_[i]->x <= x )
    {
      if ( count_-1 == i )
	{
	  retPix.j = picturePtr_[i]->y;
	  return retPix;
	}
      i++;
    }
  retPix.j = INT_MIN;
  for (int j=0; j<count_; j++) for (int i=j; i<count_; i++)
    {
      int index = j*count_+i;
      float *vDist = line[index]->vertDistP(x, 0);
      if ( vDist )
	{
	  retPix.j = MAX( (int) rint(*vDist), retPix.j);
	  zap(vDist);
	}
    }
  // Average lines with a locally present pixel
  if ( picturePtr_[i-1]->x == x )
    retPix.j = (retPix.j + picturePtr_[i-1]->y) / 2;
  return retPix;
}
  
// Sort points
void TDO::sort()
{
  qsort(picturePtr_, count_, sizeof(Pixel*), (cmp_func)comparePixelPtr);
}

// Calculate intensity white
int TDO::isWhiter()
{
  if ( value_ >= neutral_ ) return 1;
  else return 0;
}

// Check for ball shape
int TDO::isBall(const ballTune T, int verbose)
{
  if ( 0 < onEdge_ && !loadTest_ )
    {
#ifdef VERBOSE
      if ( 2 < verbose ) cout << "on Edge : " << *this << flush;
#endif
      return 0;
    }
  if ( 0 < onCorner_ )
    {
#ifdef VERBOSE
      if ( 5 < verbose ) cout << "on Corner : " << *this << flush;
#endif
      return 0;
    }
  if ( T.minBallDensity > density_ )
    {
#ifdef VERBOSE
      if ( 2 < verbose ) cout << "density too small : " << *this << flush;
#endif
      return 0;
    }
  if ( T.minBallRatio   > ratio_ )
    {
#ifdef VERBOSE
      if ( 2 < verbose ) cout << "ratio too small : " << *this << flush;
#endif
      return 0;
    }
  if ( T.maxBallRatio   < ratio_ && T.maxBallRatio < 120.0 )
    {
#ifdef VERBOSE
      if ( 2 < verbose ) cout << "ratio=" << ratio_ << " too large (" 
			      << T.maxBallRatio << " : " << *this << flush;
#endif
      return 0;
    }
  if ( T.minBallArea    > area_ )
    {
#ifdef VERBOSE
      if ( 2 < verbose ) cout << "area too small : " << *this << flush;
#endif
      return 0;
    }
  if ( T.maxBallArea    < area_      )
    {
#ifdef VERBOSE
      if ( 2 < verbose ) cout << "area too large : " << *this << flush;
#endif
      return 0;
    }
  else
    {
#ifdef VERBOSE
      if ( 2 < verbose ) cout << "ball : " << *this << flush;
#endif
      return 1;
    }
}

// Calculate properties for a segment
void TDO::calcProps()
{
  int nLine  = count_*count_;
  GridLine **line;      // segments connecting pixels

  // Do this here because compiler unhappy with constructor placement
  loadTest_ = image_->loadTest();

  // Calculate value
  int sum = 0;
  for (int j=0; j<count_; j++)
    {
      onEdge_   += picturePtr_[j]->onEdge;
      onCorner_ += picturePtr_[j]->onCorner;
      sum       += picturePtr_[j]->value();
    }
  onEdge_   = MIN( onEdge_, 1);
  onCorner_ = MIN( onCorner_, 1);
  value_    = (float) sum / (float) count_;

  // Deal with the corners and return
  if ( onCorner_ )
    {
      centroid_.x = 0.0;
      centroid_.y = 0.0;
      imin_       =   0;
      imax_       =   0;
      jmin_       =   0;
      jmax_       =   0;
      ratio_      =   0;
      density_    =   0;
      return;
    }

  // Define line segments connecting all the points
  line = new GridLine *[nLine];
  for (int j=0; j<count_; j++) for (int i=j; i<count_; i++)
    {
      line[j*count_+i] =
	new GridLine (picturePtr_[i]->x, picturePtr_[i]->y,
			  picturePtr_[j]->x, picturePtr_[j]->y, distance_);
    }
  for (int j=0; j<count_; j++) for (int i=0; i<j; i++)
    {
      line[j*count_+i] = new GridLine();
    }
  
  // Draw polygon around shape
  imin_          = picturePtr_[0]->x;
  imax_          = picturePtr_[count_-1]->x;
  int nPer       = imax_ - imin_ + 1;
  pixelLoc *pTop = new pixelLoc[nPer];
  pixelLoc *pBot = new pixelLoc[nPer];

  // find tops and bottoms
  for (int i=0, ind=imin_; i<nPer; i++, ind++)
    {
      pBot[i] = bottom(ind, line);
      pTop[i] = top(ind, line);
    }

  // Calculate area, top edge, and bottom edge
  // Correct for aberration only in centroid calculation
  float x, y;
  float iminf = (float)imin_;
  jmin_ = INT_MAX;
  jmax_ = INT_MIN;
  area_ = 0;
  centroid_.x = 0;
  centroid_.y = 0;
  for (int i=0; i<nPer; i++)
    {
      x     = float( i + imin_ );
      y     = float( pTop[i].j + pBot[i].j ) / 2.0;
      (image_->CAL())->CameraData::correctRx(&x, &y);
      jmin_ = MIN(jmin_, pBot[i].j);
      jmax_ = MAX(jmax_, pTop[i].j);
      int  width = pTop[i].j - pBot[i].j + 1;
      area_ += width;
      centroid_.x += (x + 1.0 - iminf) * float( width );
      centroid_.y += y * float( width );
    } 
  centroid_.x  = centroid_.x / area_ + imin_ - 1;
  centroid_.y  = centroid_.y / area_;
  if ( calibrate )  // support for measureRaw
    {
      centroidDist_.x = centroid_.x;
      centroidDist_.y = centroid_.y;
      (image_->CAL())->CameraData::distort(&(centroidDist_.x),
					   &(centroidDist_.y));
    }
  int leftj  = (pBot[0].j      + pTop[0].j     ) / 2;
  int rightj = (pBot[nPer-1].j + pTop[nPer-1].j) / 2;
  ratio_ = (float)area_ / nPer / distance_->value(imax_-imin_, rightj-leftj);
  density_     = (float) count_ / (float) area_ * (float) image_->sub();

  // Delete line & perimeter information
  for (int j=0; j<nLine; j++) if ( line[j] ) zap(line[j]);
  if ( line ) zaparr(line);
  zaparr(pTop);
  zaparr(pBot);
  return;
}

// Print picture
ostream & operator<< (ostream & out, const TDO &O)
{
  char outstr[MAX_CANON];
  if ( calibrate )  // support for measureRaw
    {
      //      if ( 1 < verbose )
      //	sprintf(outstr,
      //	      "val=%4.0f, area=%3d, dens=%4.2f, BallRaw Pixels:  %5.1f,%5.1f\n",
      //		O.value_, O.area_, O.density_,
      //		O.centroidDist_.x, O.centroidDist_.y);
      // else
	sprintf(outstr,  "val=%4.0f, area=%3d, dens=%4.2f, %5.1f,%5.1f Rx, BallRaw Pixels: %5.1f,%5.1f\n",
		O.value_, O.area_, O.density_, O.centroid_.x, O.centroid_.y,
		O.centroidDist_.x, O.centroidDist_.y);
    }
  else
    {
      sprintf(outstr,
    "val=%4.0f, cnt=%3d, area=%3d, dens=%4.2f, cent=%5.1f,%5.1f Rx\n",
     O.value_, O.count_, O.area_, O.density_, O.centroid_.x, O.centroid_.y);
    }
  out << outstr << flush;
  return out;
}



////////////////////class SegmentSet/////////////////////////////////////
  // Reset storage each pass
void SegmentSet::resetNodes()
{
  nS_ = 0;
  for (int i=0; i<size_; i++)s_[i]->reset();
}

// Load a new node into separate segments
void SegmentSet::loadNode(int j, Pixel *p)
{
  nS_       = MAX(nS_, j+1);
  s_[j]->loadObjPixel(p);
}

// Calculate properties for segments
void SegmentSet::calcProperties()
{
  for (int j=0; j<nS_; j++)
    {
      s_[j]->sort();
      s_[j]->calcProps();
    }
}

// Sort
void SegmentSet::sort()
{
  qsort(s_, nS_, sizeof(TDO*), (cmp_func)compareTDOptr);
}

// Consolidate close objects that would not segregate due to clean image.
void SegmentSet::consolidate(ballTune C)
{
  /*
    Use of agauss means noise may cause join with nearby object.
    Fundamental problem of eigenvector method: a clean image
    will spread the eigenvector over whole range.
  */
  for (int I=0; I<(nS_-1); I++)  for (int i=I+1, j=I+1; i<nS_; i++, j++)
    {
      if ( s_[I]->isBall(C, 0) && s_[i]->isBall(C, 0) )
	{
	  // Determine four corners each ball
	  pixelLocf inn;
	  pixelLocf ixn;
	  pixelLocf inx;
	  pixelLocf ixx;
	  inn.x = s_[i]->imin();
	  inn.y = s_[i]->jmin();
	  ixn.x = s_[i]->imax();
	  ixn.y = s_[i]->jmin();
	  inx.x = s_[i]->imin();
	  inx.y = s_[i]->jmax();
	  ixx.x = s_[i]->imax();
	  ixx.y = s_[i]->jmax();
	  pixelLocf Inn;
	  pixelLocf Ixn;
	  pixelLocf Inx;
	  pixelLocf Ixx;
	  Inn.x = s_[I]->imin();
	  Inn.y = s_[I]->jmin();
	  Ixn.x = s_[I]->imax();
	  Ixn.y = s_[I]->jmin();
	  Inx.x = s_[I]->imin();
	  Inx.y = s_[I]->jmax();
	  Ixx.x = s_[I]->imax();
	  Ixx.y = s_[I]->jmax();
	  // Compare min corners to max corners of each other
	  if ( magErr(inn, Ixx) < C.minProx
	       || magErr(inn, Ixn) < C.minProx
	       || magErr(Inn, ixx) < C.minProx
	       || magErr(Inn, Ixn) < C.minProx
	       || magErr(s_[I]->centroid(), s_[i]->centroid()) <C.minProx   )
	    {
	      s_[I]->absorbOther(*s_[i]);
	      for (int m=i; m<(nS_-1); m++) *s_[m] = *s_[m+1];
	      nS_--; i--;
	    }
	}
    }
}

// Stream operator
ostream & operator<< (ostream & out, const SegmentSet &SS)
{
  out << SS.nS_ << " segments in SegmentSet\n";
  for (int j=0; j<SS.nS_; j++)
    {
      out << "Segment " << j << ":" << endl;
      out << "s_.count_=" << SS.s_[j]->count() << endl;
      for (int i=0; i<SS.s_[j]->count(); i++)
	{
	  out << SS.n_*(SS.s_[j]->picturePtr(i))->y
	    + (SS.s_[j]->picturePtr(i))->x
	      << "," << (SS.s_[j]->picturePtr(i))->velrate() << "; "; 
	}
      out << endl;
      out << *SS.s_[j];
    }
  return out;
}

