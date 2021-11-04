// Manage coordinates for image maniupulation functions
// D Gutz
/* 	$Id: PixelLoc.h,v 1.2 2007/02/13 21:53:19 davegutz Exp davegutz $	 */
#ifndef PixelLoc_h
#define PixelLoc_h 1
using namespace std;
#include <common.h>
#include <second.h>
#include <iomanip>     // setprecision etc

// Image coordinate pixel location functions
struct pixelLocf
{
  float x;
  float y;
};
pixelLocf apixelLocf(const float x, const float y);
float magErr(const pixelLocf A, const pixelLocf B);
pixelLocf operator-  (const pixelLocf A, const pixelLocf B);
pixelLocf operator+  (const pixelLocf A, const pixelLocf B);
pixelLocf operator*  (const pixelLocf A, float S);
pixelLocf operator/  (const pixelLocf A, float S);
pixelLocf operator*  (float S, const pixelLocf A);
ostream & operator<< (ostream & out, const pixelLocf P);

////////////////////////class PixelLoc3/////////////////////////////////////
// Three dimensional coordinate class
class PixelLoc3 : public Vector <float>
{
 protected:
 public:
  PixelLoc3() : Vector <float> (3)
    {
      (*this)[0] = 0.0;
      (*this)[1] = 0.0;
      (*this)[2] = 0.0;
    }
  PixelLoc3(int i, int j, int k) : Vector <float> (3)
    {
      (*this)[0] = (float) i;
      (*this)[1] = (float) j;
      (*this)[2] = (float) k;
    }
  PixelLoc3(float x, float y, float z) : Vector <float> (3)
    {
      (*this)[0] = x;
      (*this)[1] = y;
      (*this)[2] = z;
    }
  PixelLoc3(const pixelLocf & P) : Vector <float> (3)
    {
      (*this)[0] = P.x;
      (*this)[1] = P.y;
      (*this)[2] = 0.0;
    }
  PixelLoc3(const Vector <float> & V) : Vector <float> (3)
    {
      (*this)[0] = V[0];
      (*this)[1] = V[1];
      (*this)[2] = V[2];
    }
  PixelLoc3(const PixelLoc3 & P) : Vector <float> (3)
    {
      (*this)[0] = P[0];
      (*this)[1] = P[1];
      (*this)[2] = P[2];
    }
  PixelLoc3(const pixelLocf & P, const float & foc) : Vector <float> (3)
    {
      (*this)[0] = P.x;
      (*this)[1] = P.y;
      (*this)[2] = foc;
    }
  ~PixelLoc3(){};
  PixelLoc3 & operator=(const PixelLoc3 & P)
    {
      if(this==&P)return *this;  // Allows self-assignment.
      (*this)[0] = P[0];
      (*this)[1] = P[1];
      (*this)[2] = P[2];
      return *this;  // Allows stacked assignments.
    }
  PixelLoc3 & operator=(const Vector <float> & P)
    {
      if(this==&P)return *this;  // Allows self-assignment.
      (*this)[0] = P[0];
      (*this)[1] = P[1];
      (*this)[2] = P[2];
      return *this;  // Allows stacked assignments.
    }
  void norm()
    {
      float alpha = 0;
      for (int i=0; i<(*this).dim(); i++) alpha += (*this)[i]*(*this)[i];
      alpha = sqrt(alpha);
      for (int i=0; i<(*this).dim(); i++) (*this)[i]  /= alpha;
    }
  void scale(const float S)
    {
      (*this)[0] *= S;
      (*this)[1] *= S;
      (*this)[2] *= S;
    }
  friend ostream & operator<< (ostream & out, const PixelLoc3 P)
    {
      out <<  setprecision(6) << setw(11) << P[0] << " "
	   << setprecision(6) << setw(11) << P[1] << " "
	   << setprecision(6) << setw(11) << P[2];
      return out;
    }
  friend void center(PixelLoc3 & P, int W, int H)
    {
      cerr << "WARNING(PixelLoc.h): using center(PixelLoc3 & P, int W, int H)\n"
	   << "without scaling correction. Use  center(PixelLoc3 & P,\n"
	   << "`float im, float jm) instead." << endl;
      P[0] = P[0] - float(W-1) / 2.0;
      P[1] = float(H-1)/2.0 - P[1];
    }
  friend void center(PixelLoc3 & P, int W, int H, float ARC)
    {
      float im = float(W-1) / 2.0;
      float jm = float(H-1) / 2.0;
      P[0] = P[0]  - im;
      P[1] = jm    - P[1];
    }
  friend void center(PixelLoc3 & P, float im, float jm)
    {
      P[0] = P[0] - im;
      P[1] = jm - P[1];
    }
  friend void rotate(PixelLoc3 & P, const Matrix <float> A)
    {
      P = A*P;
    }
};

#endif // PixelLoc_h







