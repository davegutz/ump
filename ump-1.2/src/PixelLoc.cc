// Manage coordinates for image maniupulation functions
// D Gutz
// 	$Id: PixelLoc.cc,v 1.1 2007/02/10 00:27:52 davegutz Exp davegutz $	

// Includes section
#include "PixelLoc.h"

pixelLocf apixelLocf(const float x, const float y)
{
  pixelLocf temp;
  temp.x = x;
  temp.y = y;
  return (temp);
};

// norm of difference
float magErr(const pixelLocf A, const pixelLocf B)
{
  float dx = A.x - B.x;
  float dy = A.y - B.y;
  float error = sqrt(dx*dx + dy*dy);
  return error;
}

// Vector subtraction
pixelLocf operator-(const pixelLocf A, const pixelLocf B)
{
  pixelLocf C = {A.x - B.x, A.y - B.y};
  return C;
}

// Vector addition
pixelLocf operator+(const pixelLocf A, const pixelLocf B)
{
  pixelLocf C = {A.x + B.x, A.y + B.y};
  return C;
}

// Vector multiplication
pixelLocf operator*(const pixelLocf A, float S)
{
  pixelLocf C = {A.x * S, A.y * S};
  return C;
}

// Vector division
pixelLocf operator/(const pixelLocf A, float S)
{
  pixelLocf C = {A.x / S, A.y / S};
  return C;
}

// Scalar multiplication
pixelLocf operator*(float S, const pixelLocf A)
{
  pixelLocf C = {A.x * S, A.y * S};
  return C;
}

// Output friend
ostream & operator<< (ostream & out, const pixelLocf P)
{
  out << P.x << " " << P.y;
  return out;
}

