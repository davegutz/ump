// Calculate 3D location given calibration of two cameras and pixel
// locations in each camera image
// 	$Id: triangulate.cc,v 1.2 2006/08/23 23:18:28 davegutz Exp $	

// Includes section
using namespace std;
#include <common.h>
#ifdef HAVE_GETOPT_LONG
 #include <getopt.h>   // Argument list handling
#endif
#include <math.h>      // math
#include <string>      // string classes
#include <string.h>    // strings
#include <iostream>    // cerr etc
#include <iomanip>     // setprecision etc
#include <new>         // assertion
#include <sys/param.h> // built-in constants and MIN/MAX
#include <assert.h>    // assertion
#include <limits.h>    // access built-in constants
#include <second.h>    // local brings in TNT stuff
#include <stdio.h>     // sprintf
#include "PixelLoc.h"  // rotate, center, etc
#include "Tune.h"      // Calibration input
#include <sys/time.h>  // timing
#include <zap.h>

// Globals
int verbose = 0;       // print lots of information
int raw     = 0;       // assume raw pixel input
timeval globalProcStart;           // Process start time
int   dump              =   0;     // Dump debug command
int   calibrate         =   0;     // Doing calibration

// Program usage
void usage(char *prog, int width, int height)
{
    char *h;

    if ( NULL != (h = strrchr(prog,'/')) )
	prog = h+1;
    fprintf(stdout,
	    "%s Takes raw pixel coordinates, first left, then right,\n"
	    "corrects for distortion, then triangulates position.\n\n"
	    "usage:\n %s [ options ]\n"
	    "\n"
	    "options:\n"
	    "  -h, --help         this screen                     \n"
	    "  -s, --size         capture WIDTHxHEIGHT     [%dx%d]\n"
	    "  -r, --raw          assume raw pixel input      [%d]\n"
	    "  -V, --version      print version                   \n"
	    "  -v, --verbose      print lots of information   [%d]\n"
	    "                                                     \n"
	    "examples:                                            \n"
	    "%s -s 80x60 51 40 45 67                              \n"
	    "%s -s 80x60 -r 51 40 45 67                           \n"
	    "--                                                   \n"
	    "(c) 2005 Dave Gutz <davegutz@alum.mit.edu>           \n",
	    prog, prog, width, height, raw, verbose, prog, prog);
}

/////////////////////main/////////////////////////////////////////
int main(int argc, char **argv)
{
  int   width0  = 0;
  int   height0 = 0;
  int   width1  = 0;
  int   height1 = 0;
  char c;
 
  if ( 0 == argc )
    {
      usage(argv[0],  width0, height0);
      exit(1);
    }

  // parse options
  int num_args=0;
  while (1)
    {
#ifdef HAVE_GETOPT_LONG
      int option_index = 0;
      static struct option long_options[] =
      {
	{"help",        0, 0, 'h'},
	{"size",        1, 0, 's'},
	{"raw",         0, 0, 'r'},
	{"version",     0, 0, 'V'},
	{"verbose",     1, 0, 'v'},
	{0, 0, 0, 0}
      };
      c = getopt_long(argc, argv, "hrs:Vv:", long_options, &option_index);
#else
      c = getopt(argc, argv, "hrs:Vv:");
#endif // HAVE_GETOPT_LONG
      if ( -1 == c )
	{
	  if ( 3 < num_args )
	    {
	      usage(argv[0],  width0, height0);
	      exit(1);
	    }
	  break;
	}
      num_args++;
      switch (c) 
	{
	case 's':
	  if ( 2 != sscanf(optarg,"%dx%d", &width0, &height0) )
	    width0 = height0 = 0;
	  width1 = width0; height1 = height0;
	  break;
	case 'r':
	  raw = 1;
	  break;
	case 'V':
	  cout << "triangulate:	$Id: triangulate.cc,v 1.2 2006/08/23 23:18:28 davegutz Exp $" << endl
	       << "written by Dave Gutz Copyright (C) 2005"   << endl;
	  exit(0);
	  break;
	case 'v':
	  verbose = atoi(optarg);
	  if ( 0 > verbose )
	    {
	      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__<< " : "  
		   << "bad verbose specified\n";
	      exit(1);
	    }
#ifndef VERBOSE
	  if ( verbose ) cerr << "WARNING(ump) : recompile \'-DVERBOSE\' to "
			      << "get full verbose messages.\n" << flush;
#endif 
	  break;
	case 'h':
	  usage(argv[0],  width0, height0);
	  exit(0);
	  break;
	default:
	  usage(argv[0],  width0, height0);
	  if ( 2 > num_args ) exit(1);
	  break;
	}
    }
  if ( argc < 5 )
    {
      usage(argv[0],  width0, height0);
      exit (1);
    }


  // Trap errors
  if ( 0 == width0 || 0 == height0 || 0 == width0 || 0 == height1 )
    {
      cerr << "ERROR(triangulate):  Image Sizes wrong.\nImage Size 0=" << width0
	   << "x" << height0 << ".  Image Size 1=" << width1 << "x" << height1
	   << "." << endl;
      exit(1);
    }

  // Get calibration
  Calibration *Ca0 = new Calibration("video0.cal", &width0, &height0);
  if ( verbose )
    {
      cout << "Image Size 0=" << width0 << "x" << height0 << endl;
      cout << "Ca0=" << *Ca0;
    }
  Calibration *Ca1 = new Calibration("video1.cal", &width1, &height1);

  if ( verbose )
    {
      cout << "MESSAGE(triangulate):  Image Size 1=" << width1
	   << "x" << height1 << endl;
      cout << "Ca1=" << *Ca1;
    }

  // Get input and correct for aberration if necessary
  if ( verbose )
    {
      cout << "argc=" << argc <<endl;
      cout << " argv=";
      for (int i=0; i<argc; i++)
	{
	  cout << argv[i] << ", ";
	}
      cout << endl;
    }
  pixelLocf c0 = {atof(argv[argc-4]), atof(argv[argc-3])};
  pixelLocf c1 = {atof(argv[argc-2]), atof(argv[argc-1])};

  if ( raw ) 
    {

      // Correct for aberration
      if ( verbose )
	{
	  cout << "Pixels from first coordinates raw= " << c0 << endl;
	  cout << "Pixels from second coordinates raw= " << c1 << endl;
	}

      Ca0->initCorrectRx(width0, height0);
      Ca1->initCorrectRx(width1, height1);
      Ca0->correctRx( &c0 );
      Ca1->correctRx( &c1 );
      if ( verbose )
	{
	  cout << "Pixels from first coordinates corrected to " << c0 << endl;
	  cout << "Pixels from second coordinates corrected to " << c1 << endl;
	}

      // Correct for register error
      Ca0->correctRg(&(c0.x), &(c0.y), width0, height0);
      Ca1->correctRg(&(c1.x), &(c1.y), width1, height1);
      if ( verbose )
	{
	  cout << "Pixels from first coordinates re-registered to " << c0
	       << endl;
	  cout << "Pixels from second coordinates re-registered to " << c1
	       << endl;
	}
    }

  // Scale to external calibration size
  cout << "Assuming image 0 size is " << width0 << "x" << height0 << endl;
  c0.x *= (float)Ca0->imWidthCalEx()  / (float)width0;
  c0.y *= (float)Ca0->imHeightCalEx() / (float)height0;
  cout << "Assuming image 1 size is " << width1 << "x" << height1 << endl;
  c1.x *= (float)Ca1->imWidthCalEx()  / (float)width1;
  c1.y *= (float)Ca1->imHeightCalEx() / (float)height1;
  if ( verbose )
    {
      cout << "Pixels from first coordinates re-scaled to " << c0 << endl;
      cout << "Pixels from second coordinates re-scaled to " << c1 << endl;
    }

  // Calculate position
  PixelLoc3 location;
  PixelLoc3 uncert;
  PixelLoc3 O0p(c0, Ca0->focalCalEx());
  PixelLoc3 O1p(c1, Ca1->focalCalEx());
  Vector    <float> dP(2);
  Matrix    <float> vT(2,2);
  Vector    <float> RL(2);

  // Translate measurement pixels to coordinates and normalize
  if ( verbose )
    {
      cout << "O0p=" << O0p << endl;
      cout << "O1p=" << O1p << endl;
      cout << "im,jm0=" << Ca0->imCalEx() << ", " << Ca0->jmCalEx()<< endl;
      cout << "im,jm1=" << Ca1->imCalEx() << ", " << Ca1->jmCalEx()<< endl;
    }
  center(O0p, Ca0->imCalEx(), Ca0->jmCalEx());
  center(O1p, Ca1->imCalEx(), Ca1->jmCalEx());
  if ( verbose )
    {
      cout << "O0p =" << O0p << endl;
      cout << "O1p =" << O1p << endl;
    }
  norm(O0p);
  norm(O1p);
  if ( verbose )
    {
      cout << "O0p norm=" << O0p << endl;
      cout << "O1p norm=" << O1p << endl;
    }
  // Translate measurement pixels to real world coordinates
  rotate(O0p, *(Ca0->ACPW()));
  rotate(O1p, *(Ca1->ACPW()));
  norm(O0p);  // Renorm dag 12/17/05
  norm(O1p);  // Renorm dag 12/17/05
  if ( verbose )
    {
      cout << "O0p rotate=" << O0p << endl;
      cout << "O1p rotate=" << O1p << endl;
    }
  // Use library to
  // solve the equation vT*RL=dP where vT is 2x2 matrix of the first two
  // elements of predicted left and right direction vectors in world space,
  // dP is 1x2 matrix of the first two elements of left-right postions of
  // camera lens, and RL is to be calculated as the right and left camera
  // scalars on direction vectors to produce an intersection at the real
  // world object.  The problem is inherently over-specified with 4
  // coordinates (two directions in each direction vector ) predicting a 
  // 3 coordinate point in space.  This is used to advantage because lines
  // usually don't intersect allowing calculation of zone.  Assume average
  // of solutions is center of zone. Discard solutions whose determinant 
  // is small which means the projections of the two camera rays in that 
  // plane are nearly parallel.

  int combos[3][2] = { { 0, 1 },
		       { 0, 2 },
		       { 1, 2 } };
  float weight[3]  = { 0.,
		       0.,
		       0. }; // Weight to apply to each combo

  // Loop through all three combinations and average
  for ( int i=0; i<3; i++ )
    {
      int I = combos[i][0];
      int J = combos[i][1];

      // Difference between cameras
      dP[0] = (*Ca0->P())[I] - (*Ca1->P())[I];
      dP[1] = (*Ca0->P())[J] - (*Ca1->P())[J];
      if ( verbose )
	{
	  cout << "dP=" << dP << endl;
	}
      // Construct
      vT[0][0] =  O1p[I];
      vT[0][1] = -O0p[I];
      vT[1][0] =  O1p[J];
      vT[1][1] = -O0p[J];
      float detvT = fabs(vT[0][0]*vT[1][1]-vT[0][1]*vT[1][0]);
      if ( verbose )
	{
	  cout << "vT=" << vT << endl << "detvT=" << detvT << endl;
	}
      if ( 0.05 > detvT )
	{
	  if ( verbose ) cout << "MESSAGE(triangulate): skipping combo "
			      << I << "," << J << endl;
	  continue;
	}
       Vector <Subscript> indexT;
      RL = dP;
      LU_factor(vT, indexT);
      LU_solve (vT, indexT, RL);
      if ( verbose )
	{
	  cout << "RL=" << RL << endl;
	}
      // Predict location and add to weighted average subtotal
      weight[I] += 1.;
      weight[J] += 1.;
      float locI = (*Ca0->P())[I] + RL[1]*O0p[I];
      float locJ = (*Ca0->P())[J] + RL[1]*O0p[J];
      location[I] += locI;
      location[J] += locJ;
      if ( 0 == uncert[I] ) uncert[I]  = locI;
      else                  uncert[I] -= locI;
      if ( 0 == uncert[J] ) uncert[J]  = locJ;
      else                  uncert[J] -= locJ;

      // For debug
      if ( verbose )
	{
	  PixelLoc3 plane;
	  plane[I] = locI;
	  plane[J] = locJ;
	  cout << "plane =" << plane << endl;
	}
    }
  // Weighted average
  for (int i=0; i<3; i++)
    {
      if ( 0.0 < weight[i] )
	location[i] /= weight[i];
      else 
	location[i] = 0.0;
      if ( 1.0 == weight[i] ) 
	uncert[i] = 0.0;
      else
	uncert[i] /= weight[i];
    }


  cout << "TRIPOSITION=" << location << endl;

  // Check error
  cout << "UNCERTAINTY = " << uncert << " % units of TRIPOSITION" << endl;

  zap(Ca0);
  zap(Ca1);
}



