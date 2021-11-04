// 	$Id: sliceraw.cc,v 1.2 2007/02/11 14:30:55 davegutz Exp davegutz $	

// Includes section
using namespace std;
#include <common.h>
#include <memory_err.h>// set_new_handler
#ifdef HAVE_GETOPT_LONG
 #include <getopt.h>   // Argument list handling
#endif
#include <errno.h>     // Using perror
#include <math.h>      // math
#include <string>      // string classes
#include <string.h>    // strings
#include <iostream>    // cerr etc
#include <iomanip>     // setprecision etc
#include <new>         // assertion
#include <sys/param.h> // built-in constants and MIN/MAX
#include <assert.h>    // assertion
#include <sys/wait.h>  // waitpid
#include <limits.h>    // access built-in constants
#include <second.h>    // local brings in TNT stuff
#include <curve.h>     // local curve lookup class
#include <sys/types.h> // open/read
#include <sys/stat.h>  // open/read
#include <fcntl.h>     // open/read
#include <myTime.h>    // timing
#include <zap.h>
extern "C"
{
#include <makeargv.h>  // Argument list handling
 #include <Cwrappers.h> // wrappers
 #include <Cwrappthread.h>
}

// Program usage
void usage(char *prog)
{
    char *h;

    if ( NULL != (h = strrchr(prog,'/')) )
	prog = h+1;
    
    fprintf(stdout,
	    "%s reads raw image file(s) (.raw) and the associated timing \n"
	    "file (.tim) created by the umpire program and slices the \n"
	    "time according to -t option. \n"
	    "%s copies the settings file from first image (.set) to the \n"
	    "output file with extension changed to (.set). \n"
	    "If two image files are specified they are interleaved until \n"
	    "one file runs out.  The timing of first file is used.\n"
	    "Note: ump timing starts at t=0 before warmup.  So if save\n"
	    "X images after warmup of 1.5 seconds, time starts at 1.5.\n"
	    "usage:\n %s [ options ]\n"
	    "\n"
	    "options:\n"
	    "  -c |--concatenate  instead of blend. Time sequence \n"
	    "                     extended                        \n"  
	    "  -h |--help         this screen and quit            \n"
	    "  -i input  file     specify file name (.raw)        \n"
	    "  -o output file     specify file name (.raw)        \n"
	    "  -t begin:end       times                           \n"
	    "  -T |--total        maximum number of frames        \n"
	    "  -V |--version      print version and quit          \n"
	    "  -v |--verbose level print information              \n"
	    "                                                     \n"
	    "examples:                                            \n"
	    "%s -iyoyo.raw -oyoyoShort.raw -t 1.5:3.0             \n"
	    "--                                                   \n"
	    "(c) 2006 Dave Gutz <davegutz@alum.mit.edu>           \n",
	    prog, prog, prog, prog);
}
/* ---------------------------------------------------------------------- */


// Substring substitution.
void gsub(string *expr, const string target, const string replace)
{
  unsigned int j=0;
  while ( (j=expr->find(target)) < expr->length() )
    {
      expr->replace(j, target.length(), replace);
    }
}

// Make .set file name
string makeSetFileName(char *filename)
{
  string setfilename(filename);
  gsub(&setfilename, ".raw", ".set");
  return setfilename;
}

// Make .tim file name
string makeTimFileName(char *filename)
{
  string timfilename(filename);
  gsub(&timfilename, ".raw", ".tim");
  return timfilename;
}



/////////////////////main/////////////////////////////////////////
int main(int argc, char **argv)
{
  char c;
  char *testarg;

  // Flags for settings file control
  int      verbose           =   0;   // Level of verbosity
  int      concatenate       =   0;   // Indicator of concatenation
  int      width             =   0;   // Image width, pixels
  int      height            =   0;   // Image height, pixels
  int      fps               =   0;   // Frame rate, frames/sec
  int      absframes         =   0;   // Total frames
  char    *inFile[2]         = {new char[MAX_CANON], new char[MAX_CANON]};
  char    *outFile           =  new char[MAX_CANON];
  int      numInFiles        = 0;     // Number specified input file.
  int      numOutFiles       = 0;     // Number specified output file.
  float    beginT            = 0.0;   // Beginning of time slice.
  float    endT              = 1e32;  // End of time slice.
  int      maxFrames         = 32000; // Maximum number of frames
  char     device_digit;
  float    time              = 0.0;
  float    timeOut;
  float    dtime, dtimeOut;
  float    startSlice        =-1.0;
  float    endSlice          =-1.0;
  timeval  procStart;                 // Process start time
  timeval  presLoad;                  // Present load time
  timeval  pastLoad;
  timeval  newLoad;
  unsigned char   *image;
  FILE    *fp;            // Local file pointer
  int     *ifp;           // Input file handle
  int     *ofp;           // Output file handle

  // Initialization
  ifp    = new int[4];
  ofp    = new int[2];
  timerclear(&presLoad);
  timerclear(&procStart);
  timerclear(&pastLoad);
  timerclear(&newLoad);


  // parse options
  int num_args=0;
  while (1)
    {
#ifdef HAVE_GETOPT_LONG
      int option_index = 0;
      static struct option long_options[] =
      {
	{"concatenate", 0, 0, 'c'},
	{"help",        0, 0, 'h'},
	{"total",       1, 0, 'T'},
	{"version",     0, 0, 'V'},
	{"verbose",     1, 0, 'v'},
	{0, 0, 0, 0}
      };
      c = getopt_long(argc, argv, "chi:o:T:t:Vv:", long_options, &option_index);
#else
      c = getopt(argc, argv, "chi:o:T:t:Vv:");
#endif // HAVE_GETOPT_LONG
      if ( -1 == c )
	{
	  if ( 0 == num_args )
	    {
	      usage(argv[0]);
	      exit(1);
	    }
	  break;
	}
      num_args++;
      switch (c)
	{
	case 'c':
	  concatenate = 1;
	  break;
	case 'T':
	  maxFrames = atoi(optarg);
	  break;
	case 't':
	  if ( 2 != sscanf(optarg,"%f:%f", &beginT, &endT) || beginT > endT )
	    {
	      usage(argv[0]);
	      cerr << "ERROR(sliceraw): "
		   << __FILE__ << " : " << __LINE__ << " : "  
		   << "improper time inputs\n";
	      exit(1);
	    }      
	  break;
	case 'i':
	  numInFiles++;
	  if ( 1 == numInFiles )
	    {
	      strcpy(inFile[0], optarg);
	    }
	  else if ( 2 == numInFiles )
	    {
	      strcpy(inFile[1], optarg);
	    }
	  else
	    {
	      usage(argv[0]);
	      cerr << "ERROR(sliceraw): "
		   << __FILE__ << " : " << __LINE__ << " : "  
		   << "too many input files specified\n";
	      exit(1);
	    }
	  break;
	case 'o':
	  testarg = optarg;
	  while( *testarg )
	    {
	      if ( isdigit(*testarg) )
		{
		  usage(argv[0]);
		  cerr << "ERROR(sliceraw): "
		       << __FILE__<<" : "<<__LINE__<< " : "  
		       << "filenames must not contain a digit\n";
		  exit(1);
		}
	      testarg++;
	    }
	  numOutFiles++;
	  if ( 1 == numOutFiles )
	    {
	      strcpy(outFile, optarg);
	    }
	  else
	    {
	      usage(argv[0]);
	      cerr << "ERROR(sliceraw): " << __FILE__ << " : " << __LINE__ << " : "  
		   << "too many output files specified\n";
	      exit(1);
	    }
	  break;
	case 'v':
	  verbose = atoi(optarg);
	  if ( 0 > verbose )
	    {
	      cerr << "ERROR(sliceraw): "
		   << __FILE__ << " : " << __LINE__<< " : "  
		   << "bad verbose specified\n";
	      exit(1);
	    }
	  break;
	case 'V':
	  cout << "MESSAGE(sliceraw): $Id: sliceraw.cc,v 1.2 2007/02/11 14:30:55 davegutz Exp davegutz $ " << endl
	       << "written by Dave Gutz Copyright (C) 2005"   << endl;
	  exit(0);
	  break;
	case 'h':
	  usage(argv[0]);
	  exit(0);
	  break;
	default:
	  usage(argv[0]);
	  if ( 2 > num_args ) exit(1);
	  break;
	}
    }

  // Limits checking.
  if ( 0 > maxFrames || 32000 < maxFrames ) 
    {
      usage(argv[0]);
      cerr << "ERROR(sliceraw): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "T option out of range = " << maxFrames << "\n";
      exit(1);
    }
  if ( concatenate && 2 != numInFiles )
    {
      usage(argv[0]);
      cerr << "ERROR(sliceraw): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "need to supply two files for concatenation\n";
      exit(1);
    }
  if (!inFile[0] || !outFile)
    {
      usage(argv[0]);
      cerr << "ERROR(sliceraw): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "need input and output file\n";
      exit(1);
    }
    
  // Read information from .set file
  string setfilename = makeSetFileName(inFile[0]);
  if ( NULL == (fp = fopen(setfilename.data(), "r")) )
    {
      cerr << "ERROR(sliceraw): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "can't open " << setfilename << " for reading\n";
      exit(1);
    }
  char header[MAX_CANON];
  if ( 5 != fscanf(fp, "%d %d\n %d %d %c\n",
		   &width, &height, &fps, &absframes, &device_digit) )
    {
      cerr << strlen(header) << endl;
      cerr << "ERROR(sliceraw): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "problem reading file " << setfilename
	   << ".  Should be 5 fields\n";
      exit(1);
    }
  fclose(fp);
  dtime = dtimeOut = 1.0 / (float) fps;
  if ( '1' == device_digit )
    {
      timeOut = dtimeOut / 2.;
    }
  else
    {
      timeOut = 0.0;
    }

  // Open input files
  string inTime = makeTimFileName(inFile[0]);
  for (int i=0; i<numInFiles; i++)
    {
      if ( 0 > (ifp[2*i] = open(inTime.data(), O_RDONLY)) )
	{
	  cerr << "ERROR(sliceraw): "<< __FILE__ << " : " << __LINE__ << " : "  
	       << "can't open " << inTime << " for reading\n";
	  exit(1);
	}
      if ( 0 > (ifp[1+2*i] = open(inFile[i], O_RDONLY)) )
	{
	  cerr << "ERROR(sliceraw): "<< __FILE__ << " : " << __LINE__ << " : "  
	       << "can't open " << inFile << " for reading\n";
	  exit(1);
	}
    }
  int bufsize = width*height;
  image  = new unsigned char [bufsize];
  

  // Open output files
  string outTime = makeTimFileName(outFile);
  if ( 0 > (ofp[0] = creat(outTime.data(), 0666)) )
    {
      cerr << "ERROR(sliceraw): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "cannot open " << outTime << endl;
      exit(1);
    }
  if ( 0 > (ofp[1] = creat(outFile, 0666)))
    {
      cerr << "ERROR(sliceraw): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "cannot open " << outFile << endl;
      exit(1);
    }


  // Loop on data
  int readDone = 0;
  int num      = 0;
  int i        = 0;
  while( !readDone )
    {

      // Load image into buffer
      int            bytes_read;
      unsigned char *bp = &image[0];
      unsigned       bytes_needed = bufsize;
      while ( bytes_needed )
	{
	  bytes_read = read(ifp[1+2*i], bp, bytes_needed);
	  if ( -1 == bytes_read && EAGAIN != errno )
	    {
	      cerr << "ERROR(sliceraw): "
		   << __FILE__ << " : " << __LINE__ << " : "  
		   << "cannot read " << inFile << endl;
	      break;
	    }
	  if ( !bytes_read )
	    {
	      readDone = 1;
	      break;
	    }
	  if ( 0 < bytes_read )
	    {
	      bp           += bytes_read;
	      bytes_needed -= bytes_read;
	    }
	}
      if ( readDone )
	{
	  if ( concatenate && 1 > i )
	    {
	      i++;
	      readDone = 0;
	      continue;
	    }
	  else break;
	}

      // Read time stamp from first file only to shuffle
      long readTime[2];
      if ( 0 == i && !concatenate )
	{
	  if ( -1 == read(ifp[2*i], &readTime, 2*sizeof(long)) )
	    {
	      cerr << "ERROR(sliceraw): "
		   << __FILE__ << " : " << __LINE__ << " : "  
		   <<"cannot read " << inTime << endl;
	      break;
	    }
	  pastLoad        = presLoad;
	  presLoad.tv_sec = readTime[0];
	  presLoad.tv_usec= readTime[1];

	  // Update the counters.
	  if ( !timerisset(&procStart) ) procStart = presLoad;
	  dtime = presLoad - pastLoad;
	  time  = presLoad - procStart;
	}
      else if ( concatenate )
	{
	  time += dtime;
	  pastLoad        = presLoad;
	  timerassign(time, &presLoad);
  	  readTime[0] = presLoad.tv_sec;
  	  readTime[1] = presLoad.tv_usec;
	}
      
      // Write
      if ( time >= beginT && time <= endT && maxFrames > num )
	{
	  // Output time
	  long writeTime[2];
	  timerassign(timeOut, &newLoad);
	  writeTime[0] = newLoad.tv_sec;
	  writeTime[1] = newLoad.tv_usec;

	  if ( 1 < verbose )
	    {
	      cout << "sliceraw:  dtime, time, readTime[0], readTime[1]="
		   << dtime << " " << time << " " << readTime[0] 
		   << " " << readTime[1] << endl;
	    }
	  if ( verbose )
	    {
	      cout << "sliceraw:  dtimeOut, timeOut, writeTime[0], writeTime[1]="
		   << dtimeOut << " " << timeOut << " " << writeTime[0] 
		   << " " << writeTime[1] << endl;
	    }
	  if ( -1 == write(ofp[0], &writeTime, 2*sizeof(long)) )
	    {
	      cerr << "ERROR(sliceraw): "
		   << __FILE__ << " : " << __LINE__ << " : "  
		   << "problem writing " << writeTime << endl;
	      exit(1);
	    }
	  if ( bufsize != write(ofp[1], image, bufsize) )
	    {
	      cerr << "ERROR(sliceraw): " 
		   << __FILE__<<" : "<< __LINE__ << " : "  
		   << "cannot write " << outFile << endl;
	      exit(1);
	    }
	  if ( 0 > startSlice ) startSlice = time;
	  endSlice = time;
	  num++;
	  timeOut += dtimeOut;
	  if ( !concatenate )
	    {
	      i++;
	      if ( numInFiles == i ) i=0;
	    }
	}
    }


  // Transfer to output .set file
  char   outSetFile[MAX_CANON];
  strcpy(outSetFile, makeSetFileName(outFile).data());
  if ( NULL == (fp = fopen(outSetFile, "w")) )
    {
      cerr << "ERROR(sliceraw): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "cannot open " << outSetFile << " for writing\n";
      exit(1);
    }
  else
    {
      fprintf(fp, "%d %d\n %d %d %c\n", 
	      width, height, fps, num, device_digit);
      fclose(fp);
    }


  cerr << "sliceraw(" << time << " total sec): "
       << startSlice << ":" << endSlice
       << "  Sliced " << num << " frames.\n";
      

  // Finish up
  zaparr(image);
  for (int j=0; j<4; j++)
    {
      if ( 0 < ifp[j] ) Close(ifp[j]);
    }
  for (int j=0; j<2; j++)
    {
      if ( 0 < ofp[j] ) Close(ofp[j]);
    }
  fflush(stdout);
  cout.flush();
  return 0;
};
