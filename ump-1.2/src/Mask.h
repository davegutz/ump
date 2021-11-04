// Calibration data
// 	$Id: Mask.h,v 1.1 2007/12/16 21:14:27 davegutz Exp davegutz $
// D Gutz
#ifndef Mask_h
#define Mask_h 1
#include <Segment.h> 
#include <PixelLoc.h>  // Pixel stuff
#include <InFile.h>    // Input file class
#include <zap.h>

// Mask structure and functions
struct window
{
  int Sx;
  int Sy;
  int Dx;
  int DDx;
  int Dy;
  int DDy;
  int add;
};
int winVal(const int x, const int y, const window W);
ostream & operator<< (ostream & out, const window W);


///////////////////////Mask class///////////////////////////////
class Mask
{
 protected:
  char   *maskFile_;       // Name of mask file
  static const int MAXMASK  = 6;  // Maximum number of mask entries
  string  name_;           // Name of mask
  int     numInputLines_;  // Number requested masks, lines input from file
  window *inputLine_;      // Input requested masks
  int     numMaskVal_;     // Number of mask entries
  int    *maskVal_;        // The mask
  int    *maskSubVal_;     // The submask made from sub-sampling
  int     maxEdgePix_;     // Limit on motion pixels (throughput)
  int     printedWarning_; // Flag if warned about default mask
  InFile *maskData_;       // Raw mask data
  int     n_;
  int     m_;
  int     default_;        // Using default mask
 public:
  Mask();
  Mask(const string name, int n, int m, int maxEdgePix);
  Mask(const Mask &M);
  Mask & operator=(const Mask & M);
  virtual ~Mask()
    {
      if ( numInputLines_ )  zaparr(inputLine_);
      zaparr(maskVal_);
      zaparr(maskSubVal_);
      if ( !printedWarning_ && numInputLines_ ) zap(maskData_);
    }

  // Element access
  friend ostream & operator<< (ostream & out, const Mask &M);
  int    numMaskVal()        { return numMaskVal_;}
  int    numMaskSubVal()     { return numMaskVal_;}
  int    numInputLines()     { return numInputLines_;}
  int    numLines()          { return maskData_->numLines();}
  int    maskVal(int i);
  int   *maskValptr()        { return maskVal_; }
  int    maskSubVal(int i);
  string token(int i, int j) { return maskData_->token(i,j); }
  int    Sx(int i);
  int    Sy(int i);
  int    Dx(int i);
  int    Dy(int i);
  int    DDx(int i);
  int    DDy(int i);
  int    add(int i);

  // Functions
  void loadMasksFrom(const string fileName);
  int  applyMasksTo(Pixel **picture, int n, int m, int sub);
};

#endif  // Mask_h
