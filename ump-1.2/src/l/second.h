// Vector template functions
// D Gutz
/* 	$Id: second.h,v 1.1 2006/08/16 22:07:15 davegutz Exp $	 */
#ifndef second_h
#define second_h 1

// Includes
#include <tnt/tnt.h>
#include <tnt/vec.h>
#include <tnt/fmat.h>
#include <tnt/cmat.h>
#include <tnt/lu.h>
using namespace TNT; 

// Globals
extern int verbose;    // print lots of information

template <class Vector>
void norm (Vector & x)
{
  float alpha = 0;
  for (int i=0; i<x.dim(); i++) alpha += x[i]*x[i];
  alpha = sqrt(alpha);
  for (int i=0; i<x.dim(); i++) x[i] /= alpha;
}

template <class Vector>
float sqnorm (Vector & x)
{
  float alpha = 0;
  for (int i=0; i<x.dim(); i++) alpha += x[i]*x[i];
  return alpha;
}

// Inverse of square matrix using crude Laplace development,
// "CRC Standard Mathematical Tables, Nineteenth Edition," 
//  Cleveland, 1971, section 6.13, pg 126. 
template <class Matrix>
Matrix inv (Matrix  & A)
{
  int N = A.num_cols();

#ifdef TNT_BOUNDS_CHECK
  // Check inputs.
  int M = A.num_rows();
  if ( M != N )
    {
      cerr << "ERROR(cof):  matrix must be square\n";
      exit(1);
    }
#endif

  Matrix invA(N, N, 0.0);
  float detA = det(A);
  for (int i=0; i<N; i++) for (int j=0; j<N; j++)
    invA[i][j] = cof(A, j, i ) / detA;
  return invA;
}


// Determinant of square matrix using crude Laplace development,
// "CRC Standard Mathematical Tables, Nineteenth Edition," 
//  Cleveland, 1971, section 4.10, pg 122. 
template <class Matrix>
float det (Matrix & A)
{
  float returnVal=0;
  int N = A.num_cols();

#ifdef TNT_BOUNDS_CHECK
  // Check inputs.
  int M = A.num_rows();
  if ( M != N )
    {
      cerr << "ERROR(cof):  matrix must be square\n";
      exit(1);
    }
#endif

  // No work needed if size 1
  if ( 1 == N )
    {
      returnVal = A[0][0];
      return returnVal;
    }
  
  // Use cofactor to compute determinant
  int i=1;
  for (int j=0; j<N; j++) returnVal += A[i][j]*cof(A, i, j);

  return returnVal;
}

// Norm matrix
template <class Matrix>
void normM (Matrix & A)
{
  float alpha = 0;
  int N = A.num_cols();
  int M = A.num_rows();
#ifdef TNT_BOUNDS_CHECK
  // Check inputs.
  if ( 1 == N )
    {
      cerr << "ERROR(norm):  matrix is too small\n";
      exit(1);
    }
  if ( 1 == M )
    {
      cerr << "ERROR(norm):  matrix is too small\n";
      exit(1);
    }
#endif
  for (int i=0; i<N; i++) for (int j=0; j<M; j++) alpha += A[i][j]*A[i][j];
  alpha = sqrt(alpha);
  for (int i=0; i<N; i++) for (int j=0; j<M; j++) A[i][j] /= alpha;
}

// Cofactor of square matrix
// "CRC Standard Mathematical Tables, Nineteenth Edition," 
//  Cleveland, 1971, section 4.9, pg 121. 
template <class Matrix>
float cof (Matrix & A, const int I, const int J)
{
  float returnVal=0;
  int N = A.num_cols();

#ifdef TNT_BOUNDS_CHECK
  // Check inputs.
  int M = A.num_rows();
  if ( M != N )
    {
      cerr << "ERROR(cof):  matrix must be square\n";
      exit(1);
    }
  if ( 1 == N )
    {
      cerr << "ERROR(cof):  matrix is too small\n";
      exit(1);
    }
#endif
  
  // Assign submatrix
  Matrix subMat(N-1,N-1,0.0);
  for (int i=0, ii=0; i<N; i++)
    {
      if ( I != i )
	{
	  ii++;
	  for (int j=0, jj=0; j<N; j++)
	    {
	      if ( J != j)
		{
		  jj++;
		  subMat[ii-1][jj-1] = A[i][j];
		}
	    }
	}
    }
  returnVal = det(subMat);

  // Sign
  if ( fmod( (float)I + (float)J, (float)2.0 ) ) returnVal = -returnVal;

  return returnVal;
}

Vector <float> second(Matrix <float> &A, Vector <float> & value,
		      Vector <float> & guess0, Vector <float> & guess1,
		      const float value0);
#endif // second_h
