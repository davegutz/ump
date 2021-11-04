// 	$Id: tim2clk.cc,v 1.1 2007/02/11 14:38:40 davegutz Exp davegutz $

// Includes section
using namespace std;
#include <common.h>
#ifdef HAVE_GETOPT_LONG
 #include <getopt.h>   // Argument list handling
#endif
#include <string>      // string classes
#include <iostream>    // cerr etc
#include <fcntl.h>     // open/read
#include <myTime.h>    // timing
extern "C"
{
 #include <Cwrappers.h> // wrappers
}

// Program usage
void usage(char *prog)
{
    char *h;

    if ( NULL != (h = strrchr(prog,'/')) )
	prog = h+1;
    
    fprintf(stdout,
	    "%s reads raw timing file (.tim) created by old versions of\n"
	    "the umpire program (ump -b) and regenerates as ascii into \n"
	    "a clock file (.clk) used by new versions of the umpire    \n"
	    "program (ump). \n"
	    "usage:\n %s [ options ] inputFile (must have .tim)    \n"
	    "\n"
	    "options:\n"
	    "  -h |--help         this screen and quit             \n"
	    "  -V |--version      print version and quit           \n"
	    "  -v |--verbose level print information               \n"
	    "(c) 2005 Dave Gutz <davegutz@alum.mit.edu>            \n",
	    prog, prog);
}
/* ---------------------------------------------------------------------- */


// Substring substitution.
int gsub(string *expr, const string target, const string replace)
{
  unsigned int j=0; int count =0;
  while ( (j=expr->find(target)) < expr->length() )
    {
      count++;
      expr->replace(j, target.length(), replace);
    }
  return count;
}

// Make .tim file name
string makeClkFileName(const string filename)
{
  string *clkFilename = new string(filename);
  if ( 1 != gsub(clkFilename, ".tim", ".clk") )
    {
      cerr << "ERROR(tim2clk): "
	   << __FILE__ << " : " << __LINE__<< " : "  
	   << "bad .tim filename = " << filename << " specified\n";
      exit(1);
    }
  return *clkFilename;
}



/////////////////////main/////////////////////////////////////////
int main(int argc, char **argv)
{
  // Declarations
  int    verbose   =  0;                     // Level of verbosity
  string inFile;                             // Input file name
  string outFile;                            // Output file name
  int    ifp;                                // Input file handle
  FILE  *ofp;                                // Output file handle
  char   c;                                  // Option parsing char
  int    num_args  =  0;                     // Option parsing counter
  timeval  procStart;                        // Process start time
  timeval  presLoad;                         // Present load time
  float    time    =  0;                     // Run time

  // Initialize
  timerclear(&procStart);
  timerclear(&presLoad);

  // parse options
  while (1)
    {
#ifdef HAVE_GETOPT_LONG
      int option_index = 0;
      static struct option long_options[] =
      {
	{"help",        0, 0, 'h'},
	{"version",     0, 0, 'V'},
	{"verbose",     1, 0, 'v'},
	{0, 0, 0, 0}
      };
      c = getopt_long(argc, argv, "hVv:", long_options, &option_index);
#else
      c = getopt(argc, argv, "hVv:");
#endif // HAVE_GETOPT_LONG
      if ( -1 == c )
	{
	  break;
	}
      num_args++;
      switch (c)
	{
	case 'v':
	  verbose = atoi(optarg);
	  if ( 0 > verbose )
	    {
	      cerr << "ERROR(tim2clk): "
		   << __FILE__ << " : " << __LINE__<< " : "  
		   << "bad verbose specified\n";
	      exit(1);
	    }
	  break;
	case 'V':
	  cout << "MESSAGE(tim2clk): "
	       << "$Id: tim2clk.cc,v 1.1 2007/02/11 14:38:40 davegutz Exp davegutz $ "
	       << "\nwritten by Dave Gutz Copyright (C) 2005\n";
	  exit(0);
	  break;
	case 'h':
	  usage(argv[0]);
	  exit(0);
	  break;
	default:
	  usage(argv[0]);
	  cerr << "Here2" << endl;
	  if ( 1 < num_args ) exit(1);
	  break;
	}
    }

  // File
  if ( argc - num_args -1 != 1 )
    {
      cerr << "Here3" << argc << " " << num_args << endl;
      usage(argv[0]);
      exit(1);
    }
  inFile = argv[optind];

  // Open input files
  if ( 0 > (ifp = open(inFile.data(), O_RDONLY)) )
    {
      cerr << "ERROR(tim2clk): "<< __FILE__ << " : " << __LINE__ << " : "  
	   << "can't open " << inFile << " for reading\n";
      exit(1);
    }

  // Open output files
  outFile = makeClkFileName(inFile);
  if ( NULL == (ofp = fopen(outFile.data(), "w")) ) 
    {
      cerr << "ERROR(tim2clk): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "cannot open " << outFile << endl;
      exit(1);
    }


  // Loop on data
  int readDone  = 0;
  int readCount = 0;
  while( !readDone )
    {

      // Load time into buffer
      int       bytes_read   = 0;
      unsigned  bytes_needed = 2*sizeof(long);
      long      readTime[2];
      if ( -1 == (bytes_read = read(ifp, &readTime, bytes_needed)) )
	{
	  cerr << "ERROR(tim2clk): "
	       << __FILE__ << " : " << __LINE__ << " : "  
	       <<"cannot read " << inFile << endl;
	  break;
	}
      if ( !bytes_read )
	{
	  readDone = 1;
	  break;
	}

      // Update the counters.
      readCount++;
      presLoad.tv_sec = readTime[0];
      presLoad.tv_usec= readTime[1];
      if ( !timerisset(&procStart) ) procStart = presLoad;
      time  = presLoad - procStart;

      // Write
      if ( 0 > fprintf(ofp, "%ld %ld\n", readTime[0], readTime[1]) )
	{
	  cerr << "ERROR(tim2clk): "
	       << __FILE__ << " : " << __LINE__ << " : "  
	       << "problem writing " << outFile << endl;
	  exit(1);
	}
    }
  if ( 0 < verbose )
    {
      cout << "MESSAGE(tim2clk): " << time << " total sec for " << readCount
	   <<  " samples. (" << time/readCount << " sec)\n";
    }

      

  // Finish up
  Close(ifp);
  fclose(ofp);
  fflush(stdout);
  cerr.flush();
  cout.flush();
  return 0;
};
