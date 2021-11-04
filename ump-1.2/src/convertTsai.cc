// Use geometry data and readings of three ball positions in each image
// to determine coordinate transformation properties and write them
// to a configuration file video?.cal for each camera device.
// Author:  D. Gutz

// Includes section
using namespace std;
#include <common.h>
#ifdef HAVE_GETOPT_LONG
 #include <getopt.h>   // Argument list handling
#endif
#include <math.h>      // math
#include <string>      // string classes
#include <string.h>    // strings
#include <fstream>    // cerr etc
#include <iomanip>     // setprecision etc
#include <new>         // assertion
#include <sys/param.h> // built-in constants and MIN/MAX
#include <assert.h>    // assertion
#include <limits.h>    // access built-in constants
#include <second.h>    // local brings in TNT stuff
#include <stdio.h>     // sprintf
#include "PixelLoc.h"  // rotate, center, etc
#include "Tune.h"      // World class
#include <sys/time.h>  // timing
#include <zap.h>
int verbose = 0;       // print lots of information
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
	    "%s Converts Tsai .tam files to ump .cal files.\n"
	    "%s Must have size input, known a-priori.\n\n"
	    "usage:\n %s [ options ]\n"
	    "\n"
	    "options:\n"
	    "  -h, --help         this screen                     \n"
	    "  -s, --size         capture WIDTHxHEIGHT     [%dx%d]\n"
	    "                                                     \n"
	    "examples:                                            \n"
	    "%s -s 80x60                                          \n"
	    "--                                                   \n"
	    "(c) 2005 Dave Gutz <davegutz@alum.mit.edu>      \n",
	    prog, prog, prog, width, height, prog);
}


/////////////////////main/////////////////////////////////////////
int main(int argc, char **argv)
{
  ofstream outf;

  // Width and height must be input
  int width  = 0;
  int height = 0;
  char c;
 
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
	{"version",     0, 0, 'V'},
	{"verbose",     0, 0, 'v'},
	{0, 0, 0, 0}
      };
      c = getopt_long(argc, argv, "hs:Vv", long_options, &option_index);
#else
      c = getopt(argc, argv, "hs:Vv");
#endif // HAVE_GETOPT_LONG
      if ( -1 == c )
	{
	  if ( 0 == num_args )
	    {
	      usage(argv[0],  width, height);
	      exit(1);
	    }
	  break;
	}
      num_args++;
      switch (c) 
	{
	case 's':
	  if ( 2 != sscanf(optarg,"%dx%d", &width, &height) )
	    width = height = 0;
	  break;
	case 'V':
	  cout << "extCal: $Id: convertTsai.cc,v 1.1 2007/01/13 14:12:26 davegutz Exp davegutz $" << endl
	       << "written by Dave Gutz Copyright (C) 2005"   << endl;
	  exit(0);
	  break;
	case 'v':
	  verbose = 1;
	  break;
	case 'h':
	  usage(argv[0],  width, height);
	  exit(0);
	  break;
	default:
	  usage(argv[0],  width, height);
	  if ( 2 > num_args ) exit(1);
	  break;
	}
    }
  

  // Read in camera measurements from .dat file.
  TsaiData  *caml  = new TsaiData("video0.tam", width, height);
  cout << *caml;
  MyCamData *camld = new MyCamData(*caml);
  TsaiData  *camr  = new TsaiData("video1.tam", width, height);
  cout << *camr;
  MyCamData *camrd = new MyCamData(*camr);

  // Write cal result files
  char calFile[MAX_CANON];
  strcpy(calFile, "video0.cal");
  outf.open(calFile, ios::out);
  if ( outf.fail() )
    {
      cerr << "Calibration: " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't open file " << calFile << endl;
      return(1);
    }
  outf << *camld;
  if ( outf.fail() )
    {
      cerr << "calibrate: " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't write file " << calFile << endl;
      exit(1);
    }
  else
    cout << "\nWrote " << calFile << ".\n";
  outf.close();
  strcpy(calFile, "video1.cal");
  outf.open(calFile, ios::out);
  if ( outf.fail() )
    {
      cerr << "Calibration: " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't open file " << calFile << endl;
      return(1);
    }
  outf << *camrd;
  if ( outf.fail() )
    {
      cerr << "calibrate: " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't write file " << calFile << endl;
      exit(1);
    }
  else
    cout << "Wrote " << calFile << ".\n";
  outf.close();

  zap(caml);
  zap(camld);
  zap(camr);
  zap(camrd);
}


