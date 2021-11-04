// Calibration file input utilities
// 	$Id: Mask.cc,v 1.4 2007/02/13 21:40:24 davegutz Exp davegutz $
// D Gutz

// Includes section
using namespace std;
#include <common.h>
#include <math.h>      // math
#include <string>      // string classes
#include <string.h>    // strings
#include <iostream>    // cerr etc
#include <iomanip>     // setprecision etc
#include <new>         // assertion
#include <sys/param.h> // built-in constants and MIN/MAX
#include <assert.h>    // assertion
#include <limits.h>    // access built-in constants
#include <stdio.h>     // sprintf
#include <InFile.h>    // Input file class
#include <stringset.h> // for fileroot
#include <zap.h>
#include "Mask.h"

// Global variables
extern int verbose;


////////////////////////class Mask////////////////////////////////////
// Constructors.
Mask::Mask()
  : maskFile_(NULL), name_(""), numInputLines_(0), inputLine_(NULL), 
    numMaskVal_(0), maskVal_(NULL), maskSubVal_(NULL), maxEdgePix_(0),
    printedWarning_(0), maskData_(NULL), n_(0), m_(0), default_(0)
{}
Mask::Mask(const string name, int n, int m, int maxEdgePix)
  : maskFile_(NULL), name_(name), numInputLines_(0), inputLine_(NULL),
    numMaskVal_(0), maskVal_(NULL), maskSubVal_(NULL), maxEdgePix_(maxEdgePix),
    printedWarning_(0), maskData_(NULL), n_(n), m_(m), default_(0)
{
  int bufsize = n*m;
  inputLine_  = new window[MAXMASK];
  maskVal_    = new int[bufsize];
  maskSubVal_ = new int[bufsize];
}
Mask::Mask(const Mask &M)
  : maskFile_(NULL), name_(M.name_), numInputLines_(M.numInputLines_),
    inputLine_(NULL), numMaskVal_(M.numMaskVal_), maskVal_(NULL),
    maskSubVal_(NULL), maxEdgePix_(M.maxEdgePix_),
    printedWarning_(M.printedWarning_), maskData_(NULL),
    n_(M.n_), m_(M.m_), default_(M.default_)
{
  strcpy(maskFile_, M.maskFile_);
  int bufsize = n_*m_;
  inputLine_  = new window[MAXMASK];
  for ( int i=0; i<MAXMASK; i++ ) inputLine_[i] = M.inputLine_[i];
  maskVal_    = new int[bufsize];
  maskSubVal_ = new int[bufsize];
  for ( int i=0; i<bufsize; i++ )
    {
      maskVal_[i]    = M.maskVal_[i];
      maskSubVal_[i] = M.maskSubVal_[i];
    }
}

Mask & Mask::operator=(const Mask & M)
{
  if ( this == &M ) return *this;  // Allows self-assignment.
  maskFile_      = NULL;
  strcpy(maskFile_, M.maskFile_);
  name_          = M.name_;
  numInputLines_ = M.numInputLines_;
  inputLine_     = NULL;
  numMaskVal_    = M.numMaskVal_;
  maskVal_       = NULL;
  maskSubVal_    = NULL;
  maxEdgePix_    = M.maxEdgePix_;
  printedWarning_= M.printedWarning_;
  maskData_      = NULL;
  n_             = M.n_;
  m_             = M.m_;
  default_       = M.default_;
  int bufsize = n_*m_;
  inputLine_  = new window[MAXMASK];
  for ( int i=0; i<MAXMASK; i++ ) inputLine_[i] = M.inputLine_[i];
  maskVal_    = new int[bufsize];
  maskSubVal_ = new int[bufsize];
  for ( int i=0; i<bufsize; i++ )
    {
      maskVal_[i]    = M.maskVal_[i];
      maskSubVal_[i] = M.maskSubVal_[i];
    }
  return *this;  // Allows stacked assignments.
}

// Element access
ostream & operator<< (ostream & out, const Mask &M)
{
  out << setprecision(2) << "# Mask " << M.name_ << "\n\n";
  if ( M.default_ )
    {
      // This exists but not LineS or token
      out << M.inputLine_[0] << endl;
      out << "0    0    1    1     0    0    1\n";
    }
  else
    for(int i=0; i<M.numInputLines_; i++)
      {
	out << M.inputLine_[i] << endl;
	for (int j=0; j<M.maskData_->LineS(i).size(); j++)
	  out << setw(5) << M.maskData_->token(i, j) << " ";
      }
  return out;
}
int Mask::Sx(int i)
{ 
  if ( numInputLines_ <= i || 0 > i)
    {
      cerr << "ERROR(Mask::Sx):  " << __FILE__ << " : " << __LINE__ << " : " 
	   << "requested line #" << i << " is less than 0 \n"
	   << "or it exceeds the length of the mask input lines = "
	   << numInputLines_  << endl << flush;
      exit(1);
    }
  return inputLine_[i].Sx;
}
int Mask::Sy(int i)
{ 
  if ( numInputLines_ <= i || 0 > i)
    {
      cerr << "ERROR(Mask::Sy): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "requested line #" << i << " is less than 0 \n"
	   << "or it exceeds the length of the mask input lines = "
	   << numInputLines_  << endl << flush;
      exit(1);
    }
  return inputLine_[i].Sy;
}
int Mask::Dx(int i)
{ 
  if ( numInputLines_ <= i || 0 > i)
    {
      cerr << "ERROR(Mask::Dx):  " << __FILE__ << " : " << __LINE__ << " : "  
	   << "requested line #" << i << " is less than 0 \n"
	   << "or it exceeds the length of the mask input lines = "
	   << numInputLines_  << endl << flush;
      exit(1);
    }
  return inputLine_[i].Dx;
}
int Mask::Dy(int i)
{ 
  if ( numInputLines_ <= i || 0 > i)
    {
      cerr << "ERROR(Mask::Dy):  " << __FILE__ << " : " << __LINE__ << " : "  
	   << "requested line #" << i << " is less than 0 \n"
	   << "or it exceeds the length of the mask input lines = "
	   << numInputLines_  << endl << flush;
      exit(1);
    }
  return inputLine_[i].Dy;
}
int Mask::DDx(int i)
{ 
  if ( numInputLines_ <= i || 0 > i)
    {
      cerr << "ERROR(Mask::DDx):  " << __FILE__ << " : " << __LINE__ << " : "
	   << "requested line #" << i << " is less than 0 \n"
	   << "or it exceeds the length of the mask input lines = "
	   << numInputLines_  << endl << flush;
      exit(1);
    }
  return inputLine_[i].DDx;
}
int Mask::DDy(int i)
{ 
  if ( numInputLines_ <= i || 0 > i)
    {
      cerr << "ERROR(Mask::DDy):    " << __FILE__ << " : " << __LINE__ << " : " 
	   << "requested line #" << i << " is less than 0 \n"
	   << "or it exceeds the length of the mask input lines = "
	   << numInputLines_  << endl << flush;
      exit(1);
    }
  return inputLine_[i].DDy;
}
int Mask::add(int i)
{ 
  if ( numInputLines_ <= i || 0 > i)
    {
      cerr << "ERROR(Mask::add): " << __FILE__ << " : " << __LINE__ << " : "
	   << "requested line #" << i << " is less than 0 \n"
	   << "or it exceeds the length of the mask input lines = "
	   << numInputLines_  << endl << flush;
      exit(1);
    }
  return inputLine_[i].add;
}
int Mask::maskVal(int i)
{
  if ( numMaskVal_ <= i || 0 > i)
    {
      cerr << "ERROR(Mask::mask):  " << __FILE__ << " : " << __LINE__ << " : " 
	   <<"  requested mask #" << i
	   << " is less than 0 \nor it exceeds the length of the mask = "
	   << numMaskVal_  << endl << flush;
      exit(1);
    }
  return maskVal_[i];
}
int Mask::maskSubVal(int i)
{
  if ( numMaskVal_ <= i || 0 > i)
    {
      cerr << "ERROR(Mask::maskSubVal):  " << __FILE__ << " : " << __LINE__ 
	   << " : "  << "requested maskSubVal #" << i
	   << " is less than 0 \nor it exceeds the length of the mask = "
	   << numMaskVal_  << endl << flush;
      exit(1);
    }
  return maskSubVal_[i];
}

// Functions

void Mask::loadMasksFrom(const string fileName)
{
  if ( numInputLines_ )
    {
      zap(maskData_);
      numInputLines_ = 0;
    }
  maskData_ = new InFile(name_.data(), fileName.data(), 128, MAX_CANON);
  if ( !maskData_->load() )
    {
      if ( verbose )
	cout << "WARNING(Mask::loadMasksFrom):  " << __FILE__ << " : " 
	     << __LINE__ << " : " << " skipping empty mask in \'" << name_ 
	     << "\'\n" << flush;
    }
  else
    {
      maskData_->upcase();
      maskData_->stripComments("#");
      if ( !maskData_->stripBlankLines() )
	{
      	  cerr << "ERROR(Mask):  " << __FILE__ << " : " << __LINE__ 
	       << " : "  << "quitting" << endl << flush;
      	  exit(1);
	}
      maskData_->tokenize(" \t\n,");
      //      cerr << "maskdata after tokenize=" << maskData_->numLines() << endl<< *maskData_ << endl;
      // dag 1/14/07 disable checking maskData_->numLines() > 0
      // Check mask
      if ( !(-1 < maskData_->numLines()) )
	{
	  cerr << "ERROR(Mask::loadMasksFrom):  " << __FILE__ << " : " 
	       << __LINE__ << " : " << "no lines in \'" << name_ << "\'\n"
	       << flush;
	  exit(1);
	}
      if ( MAXMASK < maskData_->numLines() )
	{
	  cerr << "ERROR(Mask::loadMasksFrom):  " << __FILE__ << " : " 
	       << __LINE__ << " : "  << "too many mask lines (" 
	       << maskData_->numLines() << ")\n in \'" << name_ 
	       << ".\'  Limited to " << MAXMASK << endl << flush;
	  exit(1);
	}
      if ( MAXMASK*5 >= maxEdgePix_*12)
	{
	  cerr << "ERROR(Mask::loadMasksFrom):  " << __FILE__ << " : "
	       << __LINE__ << " : " << " too many masks for \'" << name_
	       << "\'\n" << flush;
	  exit(1);
	}
      for ( int i=0; i<maskData_->numLines(); i++)
	{
      	  if ( 7 != maskData_->LineS(i).size() )
	    {
	      cerr << "ERROR(Tune::loadMasksFrom): " << __FILE__ << " : "
		   << __LINE__ << " : "
		   << " wrong number entries on line of file \'" << name_
		   << "\':"  << maskData_->Line(i) << "\n"
		   << "size=" <<  maskData_->LineS(i).size()
		   << ", LineS=" << maskData_->LineS(i) << endl
		   << " check stringset.h and stringset.cc???  Bug introduced 2/07?\n"
		   << "\nFormat is: \"sx sy dx dy ddx ddy intVal(0 or 1)\"\n"
		   << "Sample mask file format:    \n"
		   << "   0    0    1    1  0 0 0  \n"
		   << ".449 .920  .06  .06  0 0 1  \n\n"
		   << "#                           \n"
		   << "#  Explanation follows:     \n"
		   << "#  First blank the screen   \n"
		   << "   0    0    1    1     0    0    0   \n"
		   << "# Now replace the alpha characters with numbers. \n"
		   << "# The top left corner of the visible area is (Sx*N, Sy*M), where \n"
		   << "# S is units of fraction of total NxM image size. \n"
		   << "# The visible area is Dx*N wide by Dy*M tall.     \n"
		   << "# The lower right corner is skewed by DDx*N and DDy*M.\n"
		   << "  Sx   Sy    Dx   Dy   DDx   DDy  1   \n"
		   << "See the info document or ump.html for description.\n"
		   << "Look at exmask.ps (included in doc) for a graphical example\n"
		   << "Get calculator to determine the fractions.  You will have chance to iterate.\n"
		   << "The left camera from umpire point of view is 0.  Right is 1.\n"
		   << "Suggested convention for A, B, C:  start at ball near home, go clockwise.\n" << flush;
	      exit(1);
	    }
	}

      // Extract data from mask
      float sx, sy, dx, dy, ddx, ddy;
      int add;
      name_ = maskData_->name();
      int loc = 0;
      if ( verbose ) cout
	<< "MESSAGE(Mask::loadMasksFrom):  new mask input from \'" << name_
	<< "\':\n"  << flush;
      while ( maskData_->numLines() > loc && numInputLines_ < MAXMASK )
	{
	  sx  = atof(maskData_->token(numInputLines_, 0).data());
	  sy  = atof(maskData_->token(numInputLines_, 1).data());
	  dx  = atof(maskData_->token(numInputLines_, 2).data());
	  dy  = atof(maskData_->token(numInputLines_, 3).data());
	  ddx = atof(maskData_->token(numInputLines_, 4).data());
	  ddy = atof(maskData_->token(numInputLines_, 5).data());
	  add = atoi(maskData_->token(numInputLines_, 6).data());

	  // Check mask input; allow all-zero mask to pass harmlessly
	  if ( 0 < sx && 0 < sy && (0.0 < ddx || 0.0 < ddy) && 0 < add )
	  if ( dx <= 0.0 || dy <= 0.0 || ddx*ddy != 0.0 )
	    {
	      cerr << "ERROR(Mask::loadMasksFrom):  " << __FILE__ << " : "
		   << __LINE__ << " : " << " bad maskinput for \'"
		   << fileName.data() << "\'\n" << flush;
	      cerr << maskData_->Line(loc) << endl;
	      exit(1);
	    }
	  inputLine_[numInputLines_].Sx = (int)( float(n_) * sx );
	  inputLine_[numInputLines_].Sy = (int)( float(m_) * sy );
	  inputLine_[numInputLines_].Dx = (int)( float(n_) * dx );
	  inputLine_[numInputLines_].Dy = (int)( float(m_) * dy );
	  inputLine_[numInputLines_].DDx =(int)( float(n_) * ddx );
	  inputLine_[numInputLines_].DDy =(int)( float(m_) * ddy );
	  inputLine_[numInputLines_].add = add;
	  if ( 4 < verbose )
	    cout << numInputLines_ << ":  "
		 << inputLine_[numInputLines_] << endl;
	  loc++;  numInputLines_++;	  
	}	  
    }

  if ( 0 ==  numInputLines_ )
    {
      // Default mask whole window
      numInputLines_    =  1;
      inputLine_[0].Sx  =  0;
      inputLine_[0].Sy  =  0;
      inputLine_[0].Dx  = n_;
      inputLine_[0].Dy  = m_;
      inputLine_[0].DDx =  0;
      inputLine_[0].DDy =  0;
      inputLine_[0].add =  1;
      if ( verbose ) 
	cout  << "WARNING(Mask::loadMasksFrom):  No mask specified.\n" 
	      << "Default, using whole picture.\n"
	      << "Mask=\n" << inputLine_[0]
	      << endl << flush;
      printedWarning_   =  1;
    }
}

// Apply mask to the pixel map
int Mask::applyMasksTo(Pixel **picture, int n, int m, int sub)
{
  // Check compatibility
  if ( n != n_ || m != m_ )
    {
      cout << "WARNING(Mask::applyMasksTo):  " << __FILE__ << " : "
	   << __LINE__ << " : " << " size of picture = " << n << "x" << m
	   << " does not equal to expected size = " << n_ << "x" << m_ 
	   << endl << flush;
    }

  // Return if empty
  if ( 0 == numInputLines_ ) return 0;

  // Calculate and assign new mask
  numMaskVal_ = 0;
  for ( int J=0; J<m; J++) for ( int I=0; I<n; I++ )
    {
      int index = J*n + I;
      int maskVal = 0;
      if ( 0==I%sub && 0==J%sub )
	for ( int i=0; i<numInputLines_; i++ )
	  {
	    int val =  winVal(I, J, inputLine_[i]);
	    if ( 0 == val ) maskVal = 0;
	    else if ( !maskVal && 0 < val ) maskVal = 1;
	  }
      if ( maskVal )
	{
	  picture[index]->inMask = 1;
	  maskVal_[numMaskVal_++]  = index;
	}
    }
  return numMaskVal_;
}



// Provide mask values
// Only one of DDx and DDy may be non-zero.
int winVal(const int x, const int y, const window W)
{
  if ( 0 == W.Sx && 0 == W.Sy && 0 == W.Dx && 0 == W.Dy && 0 == W.DDx
       && 0 == W.DDy && 0 == W.add ) return -1;
  else if ( x    >= W.Sx + (float)(y - W.Sy)/(float)W.Dy*(float)W.DDx
	    && x <= W.Sx + (float)(y - W.Sy)/(float)W.Dy*(float)W.DDx + W.Dx
	    && y >= W.Sy + (float)(x - W.Sx)/(float)W.Dx*(float)W.DDy 
	    && y <= W.Sy + (float)(x - W.Sx)/(float)W.Dx*(float)W.DDy + W.Dy   )
    return W.add;
  else return -1;
}

ostream & operator<< (ostream & out, const window W)
{
  char outstr[MAX_CANON];
  sprintf(outstr, "# Corners(%d, %d),(%d, %d),(%d, %d),(%d, %d); Value=%d",
	  W.Sx,            W.Sy,      W.Sx+W.Dx,       W.Sy,
	  W.Sx+W.DDx,      W.Sy+W.Dy, W.Sx+W.Dx+W.DDx, W.Sy+W.Dy+W.DDy,
	  W.add);
  out << outstr;
  return out;
}
