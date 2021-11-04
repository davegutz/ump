// $Id: MyMask.h,v 1.1 2007/12/16 21:14:03 davegutz Exp davegutz $
#ifndef MyMask_h
#define MyMask_h 1
#include <Mask.h>                   // local

////////////////// class MyMask, extension of Mask ////////////////

class MyMask : public Mask
{
 protected:
  Pixel        **picture_;    // Pixels
  Pixel        **picture1_;   // P6 color pixels
  Pixel        **picture2_;   // P6 color pixels
  unsigned char *image_;      // Image present value
  unsigned char  imageZ_;     // Image past value
  int            imageR_;     // Image rate
  int            n_;          // Image horizontal pixels
  int            m_;          // Image vertical pixels
  int            bufsize_;    // Buffer size for image
  int            sub_;        // Subsample step, nominally disabled = 1
  int            useDefault_; // Use default mask
  int            useP5_;      // Gray pnm source file depth
  int            useP6_;      // Color pnm source file depth
 public:
  MyMask();
  //  MyMask(const String name, int n, int m);
  MyMask(const string name, int n, int m);
  MyMask(const MyMask & M);
  MyMask & operator=(const MyMask & M);
  virtual ~MyMask(){};

  // Write mask image to file
  //  int writeMaskImage(const String snapName, const String name);
  int writeMaskImage(const string snapName, const string name);
};

#endif  // MyMask_h
