// Find second lowest eigenvector (Fiedler vector) of matrix A given lowest
// eigenvector in guess0 and lowest eigenvector in value0. Vector value will
// contain eigenvalue solutions.  Fiedler vector returned as function return.
// Run with TNT_BOUNDS_CHECK defined to verify initial guess0 satisfies the A
// matrix for value0.
// This function is typically used for graph segmentation minimum cut
// where the lowest eigenvalue is known to be 0 and the lowest eigenvector
// is known a-priori from the eigensystem construction.
// 	$Id: second.cc,v 1.1 2006/08/16 22:07:40 davegutz Exp $	


// Includes section
using namespace std;
#include <common.h>
#include "second.h"

Vector <float> second(Matrix <float> &A, Vector <float> & value,
		      Vector <float> & guess0, Vector <float> & guess1,
		      const float value0)
{
  // Check inputs.
  int N = A.num_cols();
  if ( 2 > N )
    {
      cerr << "WARNING(second):  Cannot find second eigenvector of " << N <<
	" order system.\n";
      return Vector <float> (N, 0.0);
    }

  // Convergence tolerances
  const float tola = 1.0e-10;     // Absolute Rayleigh Quotient tolerance
  const float tolr = 0;           // Relative vector norm 
                                  // Rayleigh Quotient tolerance
  const float tola2 = 0;          // Delta Rayleigh Quotient tolerance
  const float tolr2 = 0;          // Relative delta Rayleigh Quotient tolerance
  const int   maxit = 20;         // Maximum iterations allowed
  const int   waitit= 0;          // Iterations to wait before checking
  const int   waitrs= 3;          // Iterations to wait before shifting

  // Locals
  Matrix <float> vector (2, N);   // Local values of eigenvectors.
  Vector <int>   pivot  (N, 0);   // Pivot element storage for reconstruction.
  Vector <float> v1N    (N, 0.0); // Temporary holding
  Vector <float> workN  (N, 0.0); // Temporary holding
  int    it     = 0;              // Iteration counter
  float  rq;                      // Rayleigh quotient.
  float  rqold  = 1.e10;          // Initialize Rayleigh quotient error.
  int    jpivot = 0;              // Default pivot location.
  float  shift  = value0;         // Rayleigh quotient shift for iteration.
  int NM1 = N-1;
  Vector <float> secondVec(N, 0.0);
     // Returns value of second eigenvector.

  // Pre-process guess
  for (int i=0; i<N; i++) guess1[i] -= guess0[i]*guess1[NM1];
  vector = 0.0;
  Vector <float>     work (guess0);
  Vector <float>     v1   (work);
  Matrix <float>     worka(A);
  for (int i=0; i<N; i++) worka[i][i] -= shift;

#ifdef TNT_BOUNDS_CHECK
  // Check supplied eigenvector
  const float tola0 = 1.0e-6;
  Vector <Subscript> indexT;
  LU_factor(worka, indexT);
  LU_solve (worka, indexT, v1);
  float rqtest = dot_prod(work, v1) / dot_prod(v1, v1);
  if (fabs(rqtest) > tola0)
    {
      cout
	<< "WARNING(second):  supplied first eigenvector not a good solution."
	<< "  Rayleigh quotient = " << rqtest << endl;
    }
  // Restore matrices
  worka = A;
  for (int i=0; i<N; i++) worka[i][i] -= shift;
  v1 = work;
#endif  // TNT_BOUNDS_CHECK

  // Set lowest eigenvalue.
  rq = 0;
  value[0] = shift + rq;

  // Find pivot
  float alpha = 0.;
  for (int i=0; i<N; i++)
    {
      float resid = fabs(v1[i]);
      if ( resid>alpha )
	{
	  jpivot = i;
	  alpha  = resid;
	}
    }
  pivot[0] = jpivot;

  // Swap eigenv elements, a row and col j<->nn
  work = v1;

  // Save result for deflating/inflating
  for (int i=0; i<N; i++) vector[0][i] = work[i];
  if ( jpivot!=NM1 )
    {  // If not already in correct location
      float resid  = work[jpivot];
      work[jpivot] = work[NM1];
      work[NM1]    = resid;
      alpha        = 1./resid;

      // Pivot next guess.
      resid          = guess1[jpivot];
      guess1[jpivot] = guess1[NM1];
      guess1[NM1]    = resid;

      // swaprow
      for (int k=0; k<N; k++)
	{
	  float t       = A[NM1][k];
	  A[NM1][ k]    = A[jpivot][ k];
	  A[jpivot][ k] = t;
	}
      // swapcol
      for (int k=0; k<N; k++)
	{
	  float t       = A[k][NM1];
	  A[k][NM1]     = A[k][ jpivot];
	  A[k][ jpivot] = t;
	}
    } // if jpivot
  else alpha=1./work[jpivot];
  
  // Scale and reassign
  for (int i=0; i<NM1; i++) work[i] *=alpha;
  work[NM1] = 1.;
  for (int i=0; i<NM1; i++) guess1[i] -= work[i] * guess1[NM1];
  alpha = 0;
  for (int i=0; i<NM1; i++) alpha += guess1[i]*guess1[i];
  alpha = sqrt(alpha);
  for (int i=0; i<NM1; i++) guess1[i] /= alpha;

  // Deflate
  for (int i=0; i<NM1; i++)
    {
      for (int j=0; j<NM1; j++)
	{
	  A[i][j] -= work[i] * A[NM1][j];
	}
    }

  // Second vector section
  work.newsize(NM1);
  v1.newsize(NM1);
  worka.newsize(NM1, NM1);

  //  Initialize loop
  shift = value[0];
  for (int i=0; i<work.dim(); i++) work[i] = guess1[i];
  norm(work);

  // Handle loner
  if (NM1 == 1)
    {
      value[1] = A[0][0];
      v1[0] = 1.0;
      pivot[1] = 0;
      cerr << "Going to loner\n";
    }

  // Rayleigh loop
  for (it=0; it<maxit; it++)
    {
      for (int i=0; i<NM1; i++) for (int j=0; j<NM1; j++)
	worka[i][j] = A[i][j];
      v1 = work;
      for (int i=0; i<NM1; i++) worka[i][i] -= shift;

      // Solution
      Vector <Subscript> indexT;
      if ( LU_factor(worka, indexT) )
	{
	  rq = 0;
	  goto convg;
	}
      LU_solve(worka, indexT, v1);
      //  cerr << "v1=" << v1;
      rq = dot_prod(work, v1) / dot_prod(v1, v1);
      // Normalize
      norm(v1);
      alpha = sqnorm(work);
      norm(work);
      
      if ( it >= waitit )
	{
	  // Check for termination of loop
	  if (v1[0]*work[0] < 0.0) work = v1 + work;
	  else                     work = v1 - work;
	  float rnew = sqnorm(work);
	  float deltrq = fabs(rq - rqold);
	  if (rnew*(alpha*alpha) < tolr || rnew<tola
	      || deltrq < tola2 || deltrq < tolr2*rq)
	    {
	      goto convg;
	    }
	}

      //update iteration
      rqold = rq;
      work  = v1;
      if (it>waitrs) shift += rq;  // Rayleigh quotient iteration
    }  // it<maxit
  if ( it >= (maxit-1) )
    {  // iteration count exceeded, no conv
#ifdef VERBOSE
      if ( 0 < verbose )
	{
	  cout << "WARNING(second):  no converg, eigenvalues permuted\n";
	}
#endif
      //    raise(SIGUSR1);
      return Vector <float> (N, 0.0);
    }

  // Converged
 convg:
  //cout << "second " << it << " its. \n";
  value[1] = shift + rq;
  
  // Inflate
  float enew = value[1];
  for (int i=0; i<NM1; i++) v1N[i] = v1[i];
  for (int i=0; i<N; i++) workN[i] = vector[0][i];
  float eold = value[0];
  if (eold == enew)
    {
      cout << "WARNING(second):  degenerate eigenvalues\n";
    }
  else
    {
      alpha = 0.0;
      for (int i=0; i<NM1; i++) alpha += A[NM1][i] * v1N[i];
      alpha /= (enew - eold);
      v1N[NM1]  = alpha;
      if ( pivot[0] != 0 )
	{
	  float resid = workN[pivot[0]];
	  workN[pivot[0]] = workN[NM1];
	  workN[NM1]  = resid;
	}
      alpha /= workN[NM1];

      // Norm partial v1 by new alpha (vv)
      for (int i=0; i<NM1; i++) v1N[i] = v1N[i] + workN[i] * alpha;
      float sum=0;
      for (int i=0; i<N; i++) sum += v1N[i]*v1N[i];
      sum = sqrt(sum);
      for (int i=0; i<N; i++) v1N[i] /= sum;
    }  //else


  // Permute eigenvector
  for (int i=0; i<N; i++) vector[1][i] = v1N[i];
  if ( pivot[0] )
    {
      float temp          = vector[1][pivot[0]];
      vector[1][pivot[0]] = vector[1][NM1];
      vector[1][NM1]      = temp;
    }

  // Gram-Schmidt improvement orthog. with respect to previous
  // eigenv.  Cannot do sooner easily due to permutations 
  for (int i=0; i<2; i++)
    {
      for (int j=0; j<i; j++)
	{
	  float alpha = 0.0;
	  for (int k=0; k<N; k++)
	    alpha += vector[i][k]*vector[j][k];
	  for (int k=0; k<N; k++) vector[i][k] -= alpha*vector[j][k];
	}
      float sum=0;
      for (int j=0; j<N; j++)
	sum += vector[i][j]*vector[i][j];
      sum = sqrt(sum);
      for (int j=0; j<N; j++) vector[i][j] /= sum;
    }
  
  for (int i=0; i<N; i++) secondVec[i] = vector[1][i];
  return secondVec;
}
