// 	$Id: main.cc,v 1.3 2010/01/10 11:32:51 davegutz Exp davegutz $	

// Includes section 
using namespace std;
#include <common.h>
#ifdef HAVE_GETOPT_LONG
 #include <getopt.h>   // Argument list handling
#endif
#include <iostream>    // cerr etc
#include <sys/wait.h>  // waitpid
#include <second.h>    // local brings in TNT stuff
#include <fcntl.h>     // open/read
extern "C"
{
 #include <makeargv.h>  // Argument list handling
 #include <Cwrappers.h> // wrappers
 #include <Cwrappthread.h>
}
#include "realStream.h"// bttv interface
#include <setjmp.h>    // interrupt handling
#include <sys/ioctl.h> // ioctl for video card checks
#include "PixelLoc.h"  // geometry functions
#include "Tune.h"      // calibration functions
#include "Image.h"     // Image segmentation functions
#include "Ball.h"      // Object handling functions, contains foundStuff
#include "zap.h"       // memory delete
#include <checkMem.h>  // memory checking
#define   VIDIOCGCAP   _IOR('v',1,struct video_capability)

// ----------------------------------------------------------------------
// Global variable section.
pid_t ppid;                        // Parent pid for SIGUSR1 use.
int   realTime          =   0;     // Indicator that acquiring real time
int   memTime           =   0;     // Indicator that memory mapping real time
int   signaled          =   0;     // Status of signal for broken pipe
int   dump              =   0;     // Dump debug command
int   numDev            =   0;     // Number of devices used.
int   absframes         =  45;     // Number frames for -i and -o options
int   limitingFrame     =   0;     // Attempting to limit frames
int   limitRead         =   0;     // Attempt to limit frames on a read
int   loadTest          =   0;     // Do a load test
int   writeCompact      =   1;     // Squish .ppms if doing sub-sampling
int   loadLevel         =  -1;     // No load = 0, full load = 1
int   deadband          =  30;     // Rate calc deadband
int   calibrate         =   0;     // Doing calibration
int   verbose           =   0;     // Print lots of information
int   mem               =   0;     // Global memory
int   eqCam             =   0;     // Cameras are the same from single camera
float singleCameraDist  =  48;     // Distance single camera to object, in
char  speedDisplayUnits[MAX_CANON];// Display units
timeval  globalProcStart;          // Process start time, set in Image.cc
sigset_t intmask;                  // Signal handler mask, must block globally
                                   // and use sigwait to handle.
int   pickyTime         =  0;      // Don't use new data if at same time as last
                                   // Set 0 to use latest data found
int   maxTriCount       =  0;      // Triangulation adjustment loop counter

// Shared memory control for communication with each video device (2 max)
int   smid[2]     = {       0,       0 };
int  *smptr[2]    = { new int, new int };
int   childpid[2] = {       0,       0 };

// atexit
static void stopChildren()
{
  int status;
  char msgz[MAX_CANON];
  strcpy(msgz,
       "WARNING(ump):  possible zombie because pid > childpid.  Run killUmp\n");
  if ( !memTime && !realTime ) return;
  for (int i=0; i<2; i++)
    {
      if ( childpid[i] )
	{
	  if ( 0 > kill (childpid[i], SIGQUIT) ) perror("stopChildren");
	  else
	    {
	      waitpid(childpid[i], &status, 0);
	      if ( 5 < verbose )
		{ 
		  if ( getpid() > childpid[i] )
		    write(STDERR_FILENO, msgz, strlen(msgz));
		}
	      childpid[i] = 0;
	    }
	}
    }
}
static void StopChildren(int signo)
{
  stopChildren();
  return;
}
static void freeShm()
{
  if ( !memTime && !realTime ) return;
  for (int i=0; i<numDev; i++)
    if ( smid[i] )
      {
	Shmctl (smid[i], IPC_RMID, NULL);
	cerr << "MESSAGE(ump):  freed shared memory " << smid[i] << endl;
	smid[i] = 0;
      }
}

// Status print
void printTimeResults(timeval procStart, int dev, const producerTimes local,
		      const producerTimes other, const float dtime)
{
  cout << "Motion Detected /dev/video" << local.deviceDigit << " ";
  cout << local.numMoved << "/"<< local.numNodes
       << " { ";
  if ( !timerisset(&procStart) )
    cout << "0.0" << flush;
  else
    cout << (local.presLoad - procStart)*1000.0 << flush;
  if ( 1 < numDev
       && timerisset(&(local.presLoad))
       && timerisset(&(other.presLoad)) )
    {
      cout << " skew=" 
	   << (local.presLoad - other.presLoad)*1000.0
	   << " / slips="
	   << local.nslips << flush;
    }
  cout << " }"
       << " <" 
       << 1000*(int)(local.load    - local.start)
       << "/"
       << 1000*(int)(local.motion  - local.load)
       << "/"
       << 1000*(int)(local.printed - local.motion)
       << "/"
       << 1000*(int)(local.stop    - local.printed)
       << "/"
       << 1000*(int)(local.stop    - local.start)
       << "/"
       << dtime
       << ">\n" << flush;
}

// Program usage
void usage(char *prog, char *device1, char *device2, int fps, int absframes,
	   int deadband, float warmupFrames, const float agauss, int sub,
	   int mepx, int showUndistort, const int maxTriCount, int still)
{
    char *h;

    if ( NULL != (h = strrchr(prog,'/')) )
	prog = h+1;
    
    cout 
<< "\'" << prog << "\' grabs image stream and estimates strike zone impact.\n"
<< "\'" << prog << "\'requires one of -i, -o, -R, or -M input options.\n\n"
<< "usage:\n " << prog << " [ options ]\n"
<< "\n"
<< "options:\n"
<< "  -a, --gauss        weight, pixels normalized 160 width             ["
<< agauss << "]\n"
<< "  -C, --calibrate    doing calibration                               ["
<< calibrate << "]\n"
<< "  -c, --device                             ["
<< device1 << " & " << device2 << "]\n"
<< "  -D, --deadband     deadband passed onto                           ["
<< deadband << "]\n"
<< "  -d, --dump         look for .dmp files                                 \n"
<< "  -e, --singledist   single cam to obj (in)                         [" 
<< singleCameraDist << "]\n"
<< "  -h, --help         this screen                                         \n"
<< "  -i, --in           specify file name (.raw), may have two              \n"
<< "                     on separate -i inputs                               \n"
<< "  -l, --load         do a load test, -1=none, 0=empty, 1=full       ["
<< loadLevel << "]\n"
<< "  -M, --memtime      mem mapping rt loop, # devices                      \n"
<< "  -o, --out          specify file name (.raw)                            \n"
<< "  -P, --path         specify path to .cal files                     [./] \n"
<< "  -R, --realtime     real time loop, # devices                           \n"
<< "  -r, --fps          frame rate, fps                                ["
<< fps << "]\n"
<< "  -S, --sub          subsample multiple                              ["
<< sub << "]\n"
<< "  -s, --size         capture WIDTHxHEIGHT for -R|-M         [video?.cal]\n"
<< "                     capture WIDTHxHEIGHT for -i|-o         [video?.cal]\n"
<< "                     Sizes normally from files                           \n"
<< "  -T, --tri          adjust triangulation                            ["
<< maxTriCount << "]\n"
<< "  -t, --absframes    frames after warmup                            ["
<< absframes << "]\n"
<< "  -U, --still        taking stills                                   ["
<< still << "]\n"
<< "  -u, --undistort    show undistort (corrected, Rx) in ppm           ["
<< showUndistort << "]\n"
<< "  -v, --verbose      print lots of information, levels               ["
<< verbose << "]\n"
<< "  -W, --write        write circular 100 files (.mm)                      \n"
<< "  -w, --warmup       card warmup, frames                            ["
<< warmupFrames << "]\n"
<< "  -X, --maxedgepix   max pixels normalized to 160 width            ["
<< mepx << "]\n"
<< "                                                                         \n"
<< "examples:                                                                \n"
<< "\'" << prog << " -R2\'\n"
<< "     two cameras running real-time\n"
<< "\'" << prog << " -oXA.raw -oxB.raw -t90 \'\n"
<< "     two cameras taking a three second shot\n"
<< "\'" << prog << " -R2 -W\'\n"
<< "     two cameras save 100 results to files, keep running\n"
<< "\'" << prog << " -s80x60 -t55 -r15 -ofoo.raw\'\n"
<< "     one device store file foo.raw with 55 frames at 15 fps\n"
<< "\'" << prog << " -i foo.raw\'\n"
<< "     replay the -o case from file foo.raw\n"
<< "\'" << prog << " -s80x60 -t55 -r15 -R2\'\n"
<< "     real-time, 55 frames, 15 fps\n"
<< "\'" << prog << " -R1\'\n"
<< "     single device with defaults\n"
<< "\'>temp " << prog << " -R1 -v 2\'\n"
<< "     single device with defaults, verbose and save for troubleshooting\n"
<< "\'" << prog << " -R2\'\n"
<< "     two devices with defaults\n"
<< "\'" << prog << " -R2 -c/dev/video1 -c/dev/video0\'\n"
<< "     redefine devices\n"
<< "--                                                             \n"
<< "(c) 2005 Dave Gutz <davegutz@alum.mit.edu>\n" << flush;
}
/* ---------------------------------------------------------------------- */

// For threads
pthread_t tid_myhand;
class     Ball;
sigset_t  blockmask;
void     *produce(void *);
void     *consume(void *);
void     *myhand_thread(void *);
struct nready_stuff{
  pthread_mutex_t mutex;           // mutex
  pthread_cond_t  cond;            // mutex condition
  int             producer_done;   // mutex kill flag
  int             nready;          // mutex counter
  int             numThings;       // Number of things
  producerTimes   timeSummary;     // Summary of time.
  int             dev;             // Device indicator
}nready =
#ifdef PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
 {PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP, PTHREAD_COND_INITIALIZER, 0, 0, 0};
#else
 {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0, 0};
#endif

foundStuff found[2] = {
  {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, NULL, NULL},
  {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, NULL, NULL}
};
struct int_lock_stuff{
  pthread_mutex_t mutex;           // mutex
  pthread_cond_t  cond;            // mutex condition
  int             shutdown;        // mutex kill flag
  int             snap[2];         // snap request
}int_lock = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, {0, 0}};
struct timeSlip_stuff{
  pthread_mutex_t mutex;           // mutex
  pthread_cond_t  cond;            // mutex condition
  int             lastDev;         // Previous device selected
  timeval         latest;          // Previous selected snapshot time
}timeSlip = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, -1};

// Debug synchronization
void printSyncDump(const int dev, const nready_stuff nready,
		       const char *type, const char *msg, const char *tag)
{
  if ( 5 < verbose )
    {
      if ( "C" == type ) cerr << "                              "
			      << flush;
      if ( "H" == type ) cerr << "                                       " 
			      << flush;
      if (  1  == dev  ) cerr << "              " 
			      << flush;
      cerr << msg << " " << tag
	//dag 1/8/10	   << " ( stat: " << nready.mutex.__data.__lock
	   << " nrdy: " << nready.nready << " )"
	   << endl << flush;
    }
}

// Thread calls
void acquireFound(const int dev, const char *type, const char *tag)
{
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Request found    ", tag);
#endif
  Pthread_mutex_lock (&found[dev].mutex);
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Acquired found   ", tag);
#endif
}


void acquireNready(const int dev, const char *type, const char *tag)
{
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Request nready   ", tag);
#endif
  Pthread_mutex_lock (&nready.mutex);
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Acquired nready  ", tag);
#endif
}


void acquireIntLock(const int dev, const char *type, const char *tag)
{
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Request int_lock ", tag);
#endif
  Pthread_mutex_lock(&int_lock.mutex);
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Acquired int_lock", tag);
#endif
}  


void acquireTimeSlip(const int dev, const char *type, const char *tag)
{
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Request timeSlip ", tag);
#endif
  Pthread_mutex_lock (&timeSlip.mutex);
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Acquired timeSlip", tag);
#endif
}


void releaseFound(const int dev, const char *type, const char *tag)
{
  Pthread_mutex_unlock (&found[dev].mutex);
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Released found   ", tag);
#endif
}


void releaseNready(const int dev, const char *type, const char *tag)
{
  Pthread_mutex_unlock (&nready.mutex);
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Released nready  ", tag);
#endif
}


void releaseIntLock(const int dev, const char *type, const char *tag)
{
  Pthread_mutex_unlock(&int_lock.mutex);
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Released int_lock", tag);
#endif
}  


void releaseTimeSlip(const int dev, const char *type, const char *tag)
{
  Pthread_mutex_unlock (&timeSlip.mutex);
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Released timeSlip", tag);
#endif
}


void waitNready(const int dev, const char *type, const char *tag)
{
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Wait for nready  ", tag);
#endif
  Pthread_cond_wait(&nready.cond, &nready.mutex);
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Done waiting for nready  ", tag);
#endif
}


void waitTimeSlip(const int dev, const char *type, const char *tag)
{
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Wait for timeSlip", tag);
#endif
  while ( dev == timeSlip.lastDev )
    Pthread_cond_wait(&timeSlip.cond, &timeSlip.mutex);
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Done waiting for timeSlip", tag);
#endif
}


void signalNready(const int dev, const char *type, const char *tag)
{
  Pthread_cond_signal(&nready.cond);
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Signaled nready  ", tag);
#endif
}


void signalTimeSlip(const int dev, const char *type, const char *tag)
{
  Pthread_cond_signal(&timeSlip.cond);
#ifdef VERBOSE
  printSyncDump(dev, nready, type, "Signaled timeSlip", tag);
#endif
}

/////////////////////main/////////////////////////////////////////
int main(int argc, char **argv)
{
  char c;
  pthread_t      tid_produce[2];
  pthread_t      tid_consume;
  pthread_attr_t high_prio_attr;

  // Setup threaded signal handler
  sigemptyset(&blockmask);
  sigaddset  (&blockmask, SIGINT);

  // Flags for settings file control
  Image *pix[2]            = {NULL, NULL}; // Image allocation
  int    mepx              = 200;   // Maximum pixels to try making edges
  float  agaussC           = 4.0;   // Gaussian distance weighting, calibrate
  float  agaussN           = 4.0;   // Gaussian distance weighting, normal
  float  agauss            = 0.0;   // Gaussian distance weighting, selected
  float  warmupTime        = 1.5;   // Allowed warmup, sec
  int    fps               =  30;   // Frame rate, frames/sec
  int    warmupFrames      = int(fps*warmupTime);   // Allowed warmup, sec
  float  endTime           = float(absframes) / float(fps); // End time for -i
  int    sub               =   1;   // Pixel sub-sampling multiple
  int    realSub           =   2;   // Real time sub sample
  int    lockSub           =   0;   // Indicator sub was input
  int    writeSnap         =   0;   // Write a real-time snapshot
  int    writingFile       =   0;   // Generate new file
  int    readingFile       =   0;   // Get images from file
  int    showUndistort     =   0;   // Write undistorted image to .ppm when file
  int    numDevDef         =   0;   // Counter for defining number devices
  int    still             =   0;   // Taking still shots.
  int    maxedgepix[2] =  { 0, 0};  // Sometimes recalculated local value
  Calibration *CAL[2]  =  {NULL, NULL}; // Calibration properties
  int    width[2]      =  { 0, 0};  // Image width, pixels
  int    height[2]     =  { 0, 0};  // Image height, pixels
  char  *device[2]         = {new char[MAX_CANON], new char[MAX_CANON]};// video
  char  *inFile[2]         = {new char[MAX_CANON], new char[MAX_CANON]};// -i
  char  *outFile[2]        = {new char[MAX_CANON], new char[MAX_CANON]};// -o
  char  *calPath[2]        = {new char[MAX_CANON], new char[MAX_CANON]};// -P
  int    numPath           =   0;   // Number .cal paths
  int    streampid[2]      = {0, 0};// Process id read/write stream fork
  int    status[2]         = {0, 0};// Wait status
  int   *fpi[2]            = {new int[2], new int[2]}; // pipe handles
  int   *ifd[2]            = {new int[2], new int[2]}; // pipe handles
  int   *ofd[2]            = {new int[2], new int[2]}; // pipe handles
  strcpy(speedDisplayUnits, "mph");
  strcpy(device[0], "/dev/video0");
  strcpy(device[1], "/dev/video1");
  strcpy(calPath[0], ".");
  strcpy(calPath[1], ".");
  int talk[2]              = {-1, -1};

  // parse options
  int num_args=0;
  while (1)
    {
#ifdef HAVE_GETOPT_LONG
      int option_index = 0;
      static struct option long_options[] =
      {
	{"gauss",       1, 0, 'a'},
	{"calibrate",   0, 0, 'C'},
	{"device",      1, 0, 'c'},
	{"deadband",    1, 0, 'D'},
	{"dump",        0, 0, 'd'},
	{"singledist",  1, 0, 'e'},
	{"help",        0, 0, 'h'},
	{"in",          1, 0, 'i'},
	{"load",        1, 0, 'l'},
	{"memtime",     1, 0, 'M'},
	{"out",         1, 0, 'o'},
	{"path",        1, 0, 'P'},
	{"realtime",    1, 0, 'R'},
	{"fps",         1, 0, 'r'},
	{"sub",         1, 0, 'S'},
	{"size",        1, 0, 's'},
	{"tri",         1, 0, 'T'},
	{"absframes",   1, 0, 't'},
	{"still",       0, 0, 'U'},
	{"undistort",   0, 0, 'u'},
	{"version",     0, 0, 'V'},
	{"verbose",     1, 0, 'v'},
	{"warmup",      1, 0, 'w'},
	{"write",       0, 0, 'W'},
	{"maxedgepix",  1, 0, 'X'},
	{0, 0, 0, 0}
      };
      c = getopt_long(argc, argv, "a:Cc:dD:e:hi:l:M:o:P:R:r:S:s:T:t:UuVv:Ww:X:",
		      long_options, &option_index);
#else
      c = getopt(argc, argv, "a:Cc:dD:e:hi:l:M:o:P:R:r:S:s:T:t:UuVv:Ww:X:");
#endif // HAVE_GETOPT_LONG
      if ( '?' == c ) exit(1);
      if ( -1  == c )
	{
	  if ( 0 == num_args )
	    {
	      usage(argv[0], device[0], device[1], fps, absframes, deadband,
		    warmupFrames, agaussN, sub, mepx, showUndistort,
		    maxTriCount, still);
	      exit(1);
	    }
	  break;
	}
      num_args++;
      switch (c) 
	{
	case 'a':
	  agauss = atof(optarg);
	  if ( 0 >= agauss )
	    {
	      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__<< " : "  
		   << "bad agauss input.\n";
	      exit(1);
	    }
	  break;
	case 'C':
	  calibrate = 1;
	  break;
	case 'c':
	  numDevDef++;
	  if ( 1 == numDevDef )
	    {
	      strcpy(device[0], optarg);
	    }
	  else if ( 2 == numDevDef )
	    {
	      strcpy(device[1], optarg);
	    }
	  else
	    {
	      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__<< " : "  
		   << "too many devices specified\n";
	      exit(1);
	    }
	  break;
	case 'd':
	  dump = 1;
	  break;
	case 'D':
	  deadband = atoi(optarg);
	  break;
	case 'e':
	  singleCameraDist = atof(optarg);
	  if ( 0 > singleCameraDist )
	    {
	      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__<< " : "  
		   << "bad distance specified\n";
	      exit(1);
	    }
	  break;
	case 'h':
	  usage(argv[0], device[0], device[1], fps, absframes, deadband,
		warmupFrames, agaussN, sub, mepx, showUndistort, maxTriCount,
		still);
	  exit(0);
	case 'i':
	  readingFile = 1;
	  numDev++;
	  if ( 1 == numDev )
	    {
	      strcpy(inFile[0], optarg);
	    }
	  else if ( 2 == numDev )
	    {
	      strcpy(inFile[1], optarg);
	    }
	  else
	    {
	      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__<< " : "  
		   << "too many input files specified\n";
	      exit(1);
	    }
	  break;
	case 'l':
	  loadTest = 1;
	  loadLevel = atoi(optarg);
	  break;
	case 'M':
	  numDev = atoi(optarg);
	  if ( 1>numDev || 2<numDev )
	    {
	      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__<< " : "  
		   << "device numbers (-R) out of range\n";
	      exit(1);
	    }
	  break;
	case 'o':
	  writingFile = 1;
	  numDev++;
	  if ( 1 == numDev )
	    {
	      strcpy(outFile[0], optarg);
	      strcpy(inFile[0], outFile[0]);
	    }
	  else if ( 2 == numDev )
	    {
	      strcpy(outFile[1], optarg);
	      strcpy(inFile[1], outFile[1]);
	    }
	  else
	    {
	      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__<< " : "  
		   << "too many output files specified\n";
	      exit(1);
	    }
	  break;
	case 'P':
	  numPath++;
	  if ( 1 == numPath )
	    {
	      strcpy(calPath[0], optarg);
	    }
	  else if ( 2 == numPath )
	    {
	      strcpy(calPath[1], optarg);
	    }
	  else
	    {
	      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__<< " : "  
		   << "too many cal input paths (-P) specified.  2 allowed.\n";
	      exit(1);
	    }
	  break;
	case 'R':
	  realTime = 1;
	  numDev   = atoi(optarg);
	  if ( 1>numDev || 2<numDev )
	    {
	      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__<< " : "  
		   << "device numbers (-R) out of range\n";
	      exit(1);
	    }
	  break;
	case 'r':
	  fps = atoi(optarg);
	  break;
	case 'S':
	  sub = atoi(optarg);
	  if ( 1 > sub )
	    {
	      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__<< " : "  
		   << "bad sub input.\n";
	      exit(1);
	    }
	  lockSub = 1;
	  break;
	case 's':
	  if ( 2 != sscanf(optarg,"%dx%d", &width[0], &height[0]) )
	    width[0] = height[0] = 0;
	  width[1]  = width[0];
	  height[1] = height[0];
	  break;
	case 'T':
	  maxTriCount = atoi(optarg);
	  break;
	case 't':
	  absframes = atoi(optarg);
	  limitingFrame = 1;
	  break;
	case 'U':
	  still = 1;
	  break;
	case 'u':
	  showUndistort = 1;
	  break;
	case 'V':
	  cout << "ump version 1.2-1"
	       << " written by Dave Gutz Copyright (C) 2007"   << endl;
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
	case 'W':
	  writeSnap = 1;
	  break;
	case 'w':
	  warmupFrames = atoi(optarg);
	  if ( 0 > warmupFrames )
	    {
	      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__<< " : "  
		   << "bad warmup frames specified\n";
	      exit(1);
	    }
	  break;
	case 'X':
	  mepx = atoi(optarg);
	  break;
	default:
	  usage(argv[0], device[0], device[1], fps, absframes, deadband,
		warmupFrames, agaussN, sub, mepx, showUndistort, maxTriCount,
		still);
	  if ( 2 > num_args ) exit(1);
	  break;
	}
    } // while(1)
  
  // Recalculate warmup time
  warmupTime = float(warmupFrames) / float(fps);

  // Account for warmup time when capturing to file
  if ( writingFile || loadTest || readingFile ) absframes += warmupFrames;
  
  // Consistency checking of options.
  if ( maxTriCount < 0 )
    {
      cerr << "ERROR(ump):" << __FILE__ << " : " << __LINE__ << " : "
	   << "maxTriCount=" << maxTriCount << " must be positive\n";
      exit(1);
    }
  if ( fps > 30 )
    {
      cerr << "WARNING(ump): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "requested frame rate greater than 30 may be ignored\n";
    }
  if ( 0 > mepx )
    {
      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__ << " : "  
	   << "bad edges specified, -X\n";
      exit(1);
    }
   if ( numDevDef > numDev ) 
    {
      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__ << " : "  
	   << "more devices than using\n";
      exit(1);
    }
  if ( loadTest )
    {
      if ( 0 > loadLevel || 1 < loadLevel )
	{
	  cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__ << " : "  
	       << "load level must be 0 (empty) or 1 (full)\n";
	  exit(1);
	}
    }
  if ( memTime && realTime )
    {
      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__ << " : "
	   << "incompatible -R and -M options\n";
      exit(1);
    }
  for (int i=0; i<numDev; i++)
    {
      if ( readingFile && ( 0 < width[i] || 0 < height[i] ) )
	{
	  cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__ << " : "  
	       << "cannot change image size after taken.  Use -U sub option.\n";
	  exit(1);
	}
    }
  if ( readingFile && writingFile )
    {
      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__ << " : "  
	   << "incompatible -o and -i options\n";
      exit(1);
    }
  if ( readingFile && limitingFrame ) limitRead = 1;
  if ( 1 > numDev || 2 < numDev )
    {
      usage(argv[0], device[0], device[1], fps, absframes, deadband,
	    warmupFrames, agaussN, sub, mepx, showUndistort, maxTriCount,
	    still);
      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__ << " : "  
	   << "device numbers (-R) out of range\n";
      if ( 0 == numDev ) 
	cerr << "ump requires one of -i, -o, -M, or -R options\n";
      exit(1);
    }
  if ( writeSnap && (readingFile || writingFile) )
    {
      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__ << " : "  
	   << "writing snapshot files (.mm) -W option is for -R or -M only.\n";
      exit(1);
    }
  if ( 1 < numDev && (readingFile || writingFile) )
    {
      if ( 0 == strcmp(inFile[0], inFile[1]) )
	{
	  cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__ << " : "  
	       << "input/output file names same\n";
	  exit(1);
	}
    }
  if ( !readingFile )
    {
      if ( 1 == showUndistort )
	{
	  cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__ << " : "  
	       << "cannot show file if not writing.\n";
	  exit(1);
	}
      if ( 0 == strcmp(device[0], device[1]) && 1 < numDev )
	{
	  cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__ << " : "  
	       << "devices same.\n";
	  exit(1);
	}
    }

  // agauss handling
  if ( 0 == agauss )
    {
      if ( calibrate || still ) agauss = agaussC;
      else                      agauss = agaussN;
    }


  // Set device information
  char device_digit[2];
  for (int i = 0; i < numDev; i++ ) 
    device_digit[i] = device[i][strlen(device[i])-1];
  if ( !readingFile )
    {
      char calFile[MAX_CANON];
      for (int i = 0; i < numDev; i++ )
	{
	  // Get calibration (set size = calibration)
	  sprintf(calFile,"%s/video%c.cal", calPath[i], device_digit[i]);
	  CAL[i] = new Calibration(calFile, &width[i], &height[i]);
	  if ( !CAL[i]->calFileExists() )
	    {
	      cerr << "ERROR(ump):  "<< __FILE__ << " : " << __LINE__
		   << " : " << device[i] << " : " << calFile
		   << " does not exist.  Quitting" << endl << flush;
	      exit(1);
	    }
#ifdef VERBOSE
	  if ( 4 < verbose )
	    cout << "MESSAGE(ump):  calibration parameters for device "
		 << i << ":\n" << *CAL[i] << flush;
	  
	  if ( 2 < verbose )
	    cout << "MESSAGE(ump):  image size from " << calFile
		 << " is " << width[i] << "x" << height[i] << endl << flush;
#endif

	  video_capability capability;
	  int fdvid = -1;
	  if (-1 == (fdvid = open(device[i], O_RDWR)))
	    {
	      cerr << "ERROR(ump):  "<< __FILE__ << " : " << __LINE__
		   << " : " << device[i] << " : ";
	      perror("");
	      cerr << "It appears at least one video card is not installed?\n";
	      exit(1);
	    }
	  if (-1 == ioctl(fdvid, VIDIOCGCAP, &capability))
	    {
	      cerr << "ERROR(ump):  "<< __FILE__ << " : " << __LINE__
		   << " : "  << "ioctl capability : ";
	      perror("");
	      exit(1);
	    }
	  // The 768+76 is bttv bug workaround - returns larger size
	  // than it takes
#ifdef VERBOSE
	  if ( 4 < verbose )
	    cout << "MESSAGE(ump):  asked for " << width[i] << "x" 
		 << height[i] << " for device " << i << endl << flush;
#endif
	  if ( width[i] > capability.maxwidth ||
	       width[i] < capability.minwidth ||
	       width[i] > 768+76                )
	    {
	      width[i]  = MAX(MIN(MIN(width[i], capability.maxwidth ), 768+76 ),
			                                  capability.minwidth );
	      cerr << "WARNING(ump): " << __FILE__ << " : " << __LINE__
		   << " : " << "width device " << i << " limited to "
		   << width[i] << endl;
	    }
	  if ( height[i] > capability.maxheight ||
	       height[i] < capability.minheight   )
	    {
	      height[i] = MAX( MIN( height[i], capability.maxheight ),
				  capability.minheight);
	      cerr << "WARNING(ump): " << __FILE__ << " : " << __LINE__
		   << " : " << "height device " << i << " limited to "
		   << height[i] << endl;
	    }
	  Close(fdvid);
#ifdef VERBOSE
	  if ( 4 < verbose )
	    cout << "MESSAGE(ump):  got " << width[i] << "x" 
		 << height[i] << " for device " << i << endl << flush;
#endif
	}
    }


  // Get file handle for data.
  // Read/write setfile
  if ( !(writingFile || readingFile || realTime) )
    {
      memTime = 1;
      for (int i=0; i<numDev; i++)
	{
	  // Use different subsample for real time
	  if ( !lockSub ) sub = realSub;
	  maxedgepix[i] = (int) (float(mepx) 
			      * float(width[i]*width[i]) / (160.0*160.0));
	  smptr[i] = NULL;
	  smptr[i] = pipeStreamerFork(width[i], height[i], device[i], fps,
				      absframes, deadband, &smid[i],
				      &childpid[i], ifd[i], ofd[i],
				      maxedgepix[i], smptr[i], sub, loadTest,
				      loadLevel);
	}
      atexit(freeShm);
      atexit(stopChildren);
      sleep(2);  // Wait for both streamers to get going.  
      // Block SIGINT until ready to service
      Sigprocmask(SIG_BLOCK, &blockmask, NULL);
    }
  else if ( realTime )
    {
      char INPUT[MAX_CANON];
      char TVNORM[MAX_CANON];
      strcpy(INPUT, "Composite1");
      strcpy(TVNORM, "NTSC");
      int format;
      // Block SIGINT until ready to service
      Sigprocmask(SIG_BLOCK, &blockmask, NULL);
      for (int i=0; i<numDev; i++)
	{
	  // Use different subsample for real time
	  if ( !lockSub ) sub = realSub;
	  maxedgepix[i] = (int) (float(mepx) 
			      * float(width[i]*width[i]) / (160.0*160.0));
	  smptr[i] = NULL;
	  smptr[i] = realStreamerFork(&width[i], &height[i], device[i], fps,
				      absframes, deadband, &smid[i],
				      &childpid[i], &talk[i], maxedgepix[i],
				      smptr[i], sub, loadTest, loadLevel,
				      &format, INPUT, TVNORM);
	  cerr << "MESSAGE(ump): spawned frame-grabber pid="
	       << childpid[i] << "  for device " << device[i] << endl;

	  // Recalculate in event that width/height changed by function calls
	  maxedgepix[i] = (int) (float(mepx) 
				 * float(width[i]*width[i]) / (160.0*160.0));
	}
      atexit(freeShm);
      atexit(stopChildren);
      sleep(2);  // Wait for both streamers to get going.  
    }
  else
    {
      if ( writingFile )
	{
	  for (int i=0; i<numDev; i++)
	    {
	      // Generate images
	      streampid[i] = writeStreamerFork(&device_digit[i], outFile[i],
					       &width[i], &height[i], device[i],
					       &fps, &absframes);
	    }
	  for (int i=numDev-1; i>=0; i--)
	    waitpid(streampid[i], &status[i], 0);

	  // Synchronize time for calibration runs
	  if ( calibrate && 1 < numDev )
	    {
	      char cmd[MAX_CANON];
	      string *tim0 = new string(outFile[0]);
	      gsub(tim0, ".raw", ".tim");
	      string *tim1 = new string(outFile[1]);
	      gsub(tim1, ".raw", ".tim");
	      sprintf(cmd, "cp -f %s %s", tim0->data(), tim1->data());
	      zap(tim0);
	      zap(tim1);
	      if ( 0 != Execute(cmd, 0) )
		{
		  cerr << "ERROR(ump):  "<< __FILE__ << " : " << __LINE__
		       << " : cmd : " << cmd << " failed.  Quitting"
		       << endl << flush;
		  exit(1);
		}
	    }
	}
      for (int i=0; i<numDev; i++)
	fpi[i] = readStreamerFork(&device_digit[i], inFile[i], &width[i],
				  &height[i], &fps, &absframes, limitRead);

      // Recalculate end time
      endTime    = float(absframes)    / float(fps);

      // Block SIGINT until ready to service
      Sigprocmask(SIG_BLOCK, &blockmask, NULL);
    }


  // Catch USR1 with StopChildren
  struct sigaction user;
  user.sa_handler = StopChildren;
  sigemptyset(&user.sa_mask);
  user.sa_flags = 0;
  sigaction(SIGUSR1, &user, NULL);
  ppid = getpid();

  // Using sigwait:  block SIGINT and SIGQUIT in all threads using intmask.
  sigemptyset(&intmask);
  sigaddset(&intmask, SIGINT);
  sigaddset(&intmask, SIGQUIT);
  pthread_sigmask(SIG_BLOCK, &intmask, NULL);

  // Threaded interrupt handler
  Pthread_attr_init(&high_prio_attr);
  Pthread_create(&tid_myhand, &high_prio_attr, myhand_thread, pix);


  // Body.
  if ( memTime )
    {
      for (int i=0; i<numDev; i++)
	pix[i] = new MemImage(i, width[i], height[i], fps, smptr[i],
			      device_digit[i], writeSnap, maxedgepix[i],
			      ifd[i][0], ofd[i][1], device[i], warmupTime,
			      agauss, sub, calibrate, loadTest, writeCompact,
			      CAL[i]);
    }
  else if ( realTime )
    {
      for (int i=0; i<numDev; i++)
	pix[i] = new RealImage(i, width[i], height[i], fps, smptr[i],
			       device_digit[i], writeSnap, maxedgepix[i],
			       &talk[i], device[i], warmupTime, agauss, sub,
			       calibrate, loadTest, writeCompact, CAL[i]);
    }
  else
    {
      for (int i=0; i<numDev; i++)
	{
	  // Get calibration (set size = calibration)
	  char calFile[MAX_CANON];
	  sprintf(calFile,"%s/video%c.cal", calPath[i], device_digit[i]);
	  CAL[i] = new Calibration(calFile, &width[i], &height[i]);
	  if ( !CAL[i]->calFileExists() )
	    {
	      cerr << "ERROR(ump):  "<< __FILE__ << " : " << __LINE__
		   << " : " << device[i] << " : " << calFile
		   << " does not exist.  Quitting" << endl << flush;
	      exit(1);
	    }
	  
#ifdef VERBOSE
	  if ( 4 < verbose )
	    cout << "MESSAGE(ump):  calibration parameters for device "
		 << i << ":\n" << *CAL[i] << flush;
	  
	  if ( 2 < verbose )
	    cout << "MESSAGE(ump):  image size from " << calFile << " is "
		 << width[i] << "x" << height[i] << endl << flush;
#endif

	  maxedgepix[i] = (int) (float(mepx) 
			      * float(width[i]*width[i]) / (160.0*160.0));
	  pix[i] = new FileImage(i, fpi[i], device_digit[i],
				    width[i], height[i], fps,
				    maxedgepix[i], device[i], warmupTime,
				    agauss, sub, deadband, calibrate, loadTest,
				    showUndistort, writeCompact, still,
				    CAL[i], endTime, absframes);
	}
    }

  if ( 2 == numDev ) eqCam = pix[0]->cameraSameAs(*pix[1]);
  for (int i=0; i<numDev; i++)
    Pthread_create(&tid_produce[i], NULL, produce, pix[i]);
  Pthread_create(&tid_consume, NULL, consume, NULL);
  for (int i=0; i<numDev; i++)
    Pthread_join(tid_produce[i], NULL);
  Pthread_join(tid_consume, NULL);
  cerr << endl << flush;

  for (int i=0; i<numDev; i++)
    cout << "Final dev[" << i << "] number=" << pix[i]->imageNumber() << endl;


  for (int i=0; i<numDev; i++)
    {
      if (  0 == pix[i]->imageNumber() && pix[i]->time() <= warmupTime )
	cout << "Device " << i 
	     << ":  no images because warmup time, " << warmupTime 
	     << ", longer than total time, " << pix[i]->time() << endl;
    }

  // Finish up
  freeShm();
  if ( !memTime )
    {
      for (int i=0; i<numDev; i++)
	for (int j=0; j<2; j++)
	  if ( 0 < (fpi[i])[j] ) Close((fpi[i])[j] );
    }
  for (int i=0; i<numDev; i++)
    {
      if ( pix[i] ) zap(pix[i]);
      if ( CAL[i] ) zap(CAL[i]);
    }
  for ( int i=0; i<numDev; i++ )
    {
      if ( device[i]  ) zaparr(device[i]);
      if ( inFile[i]  ) zaparr(inFile[i]);
      if ( outFile[i] ) zaparr(outFile[i]);
      if ( calPath[i] ) zaparr(calPath[i]);
      if ( fpi[i]     ) zaparr(fpi[i]);
      if ( ifd[i]     ) zaparr(ifd[i]);
      if ( ofd[i]     ) zaparr(ofd[i]);
    }
  fflush(stdout);
  cout.flush();


  // Check memory usage
#ifdef VERBOSE
  if ( 0 < verbose ) cout << "MESSAGE(ump): memory usage increased by "
			  << getMem()-mem << endl << flush;
#endif
  return 0;
};  // end main


// Catch asynchronous signals
sigjmp_buf jump_to_quit;
void catch_sigsegv(int signo)
{
  char msg1[MAX_CANON];
  char msg2[MAX_CANON];
  strcpy(msg1, "Caught SIGSEGV.  Sending SIGINT to handler.\nYou should quit\n");
  strcpy(msg2, "Caught SIGSEGV.  Sending SIGQUIT to handler.\n");
  if ( memTime )
    {
      write(STDERR_FILENO, msg1, strlen(msg1));
      fflush(stderr);
      Pthread_kill(tid_myhand, SIGINT);
    }
  else
    {
      write(STDERR_FILENO, msg2, strlen(msg2));
      fflush(stderr);
      Pthread_kill(tid_myhand, SIGQUIT);
    }
  Pthread_cond_wait(&int_lock.cond, &int_lock.mutex);
  //Pthread_mutex_unlock (&nready.mutex);
  stopChildren();
  freeShm();
  siglongjmp(jump_to_quit, 1);
}

// Producer thread.  Creates image and segments it.
void *produce(void *arg)
{
  Image* img      = (Image*)arg;
  float  SLIPTIME = 1.0 / (float)img->fps();

  // Catch asynchronous signals.
  struct sigaction act;
  act.sa_handler = catch_sigsegv;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGSEGV, &act, NULL);

  // Send mask
  img->sendMask();

  // Loop until broken
  signalTimeSlip(img->dev(), "P", "P1");
  for ( ; ; )
    {
      
      // interrupt handling

      acquireIntLock(img->dev(), "P", "P2");
      acquireNready (img->dev(), "P", "P3");

      if ( int_lock.snap[img->dev()] )
	{
	  img->startSnap();
	  int_lock.snap[img->dev()] = 0;
	}
      if ( int_lock.shutdown  || nready.producer_done >= numDev )
	{
	  nready.producer_done = numDev;

	  signalNready(img->dev(), "P", "Shutting down P4");

	  sigsetjmp(jump_to_quit, 1);

	  releaseNready (img->dev(), "P", "P5");
	  releaseIntLock(img->dev(), "P", "P6");


	  return(NULL);
	}

      releaseNready (img->dev(), "P", "P7");
      releaseIntLock(img->dev(), "P", "P8");
      

      // Load image.  Break loop if return non-zero
      if ( 1 < numDev )
	{
	  acquireTimeSlip(img->dev(), "P", "P9");
	  if ( 0 == nready.producer_done )
	    waitTimeSlip   (img->dev(), "P", "P10");

 	  timeSlip.lastDev =  img->dev();
	  img->nslips      =  0;
	}
    reload:
      if ( img->loadOne() )
	{

	  acquireNready  (img->dev(), "P", "P12");
	  nready.producer_done += 1;

	  signalNready   (img->dev(), "P", "P13");
	  releaseNready  (img->dev(), "P", "P14");
	  signalTimeSlip (img->dev(), "P", "P15");
	  releaseTimeSlip(img->dev(), "P", "P16");

	  // Cleanup memory
	  for (int j=0; j<found[img->dev()].numThings; j++)
	    if ( found[img->dev()].thingP[j] ) zap(found[img->dev()].thingP[j]);
	  if (found[img->dev()].thingP) zaparr(found[img->dev()].thingP);
	  found[img->dev()].numThings   = 0;

#ifdef VERBOSE
	  if ( 5 < verbose ) cerr << "Producer " << img->dev() 
			       << ":  returning." << endl << flush;
#endif
	  return(NULL);
	}

      // Calculate the image motion
#ifdef VERBOSE
      if ( 5 < verbose )
	{
	  if ( 1 == img->dev() ) cerr << "              " << flush;
	  cerr << "img->move produce P17 (" << img->time() 
	       << ")" << endl<< flush;
	}
#endif
      img->move();

      // Announce load test
      if ( loadTest && img->time() > img->warmupTime() ) 
	{
	  cout << img->dev() << ": " << " load test motion @ " << flush;
	  cout << img->time() << " sec\n" << flush;
	}

      // Determine if slip file image to synchronize with other device
      if ( 1 < numDev )
	{
	  if ( !memTime
	       && timerisset(&timeSlip.latest)
	       &&  SLIPTIME < (timeSlip.latest - img->presLoad()) )
	    {
	      img->nslips++;
#ifdef VERBOSE
	      if ( 5 < verbose )
		cout <<  img->dev() << " sync slip reload(" << img->time()
		     << "):" <<  img->presLoad()-timeSlip.latest
		     << endl << flush;
#endif
	      goto reload;
	    }
	    
	  timeSlip.latest = img->presLoad();

	  signalTimeSlip  (img->dev(), "P", "P18");
	  releaseTimeSlip (img->dev(), "P", "P19");

	} // if ( 1 < numDev  )

      // Segment the image
      if ( 1 == img->segment() )
	{

	  // to force consumation
	  if( memTime || realTime )
	    {
	      acquireNready  (img->dev(), "P", "P20");
	      if ( 0 < nready.nready )
		{
		  waitNready  (img->dev(), "P", "P20");
		}
	    }
	  else
	    {
	      acquireNready  (img->dev(), "P", "P20");
	      while ( 0 < nready.nready )
		{
		  waitNready  (img->dev(), "P", "P20");
		  if ( 0 < nready.nready )
		    {
		      signalNready   (img->dev(), "P", "P20");
		    }
		}
	    }


	  // Relay information
	  // ALWAYS signalNready when changing nready.
	  signalNready   (img->dev(), "P", "P21");
	  nready.nready++;
	  nready.dev = img->dev();
	  nready.timeSummary = img->timeSummary();
	  int D = nready.dev;

	  acquireFound   (D, "P", "P22, clearing old found information");

	  // Cleanup old stored information
	  for (int j=0; j<found[D].numThings; j++)
	    zap(found[D].thingP[j]);
	  if (found[D].thingP) zaparr(found[D].thingP);
	  found[D].numThings   = 0;
	  found[D].thingP      = new Ball*[img->numThings()];
	  found[D].pixP        = img;
	  //TODO:  change 2-d balls into 'discs' and things into 'blobs'
	  for (int j=0; j<img->numThings(); j++)
	    {
	      if ( (img->thingP(j))->isBall(img->ballCrit(), verbose) )
		{
		  found[D].thingP[found[D].numThings++]
		    = new Ball(img->thingP(j));
		}
	    }

	  if ( !loadTest )
	    if ( 0 == img->dev() ) cerr << ".";
	    else                   cerr << ":";

	  found[D].presLoad     = img->presLoad();
	  found[D].prevLoad     = img->prevLoad();
	  found[D].dtime        = img->dtime();
	    
	  releaseFound(D, "P", "P23, and loading new found information");
	  releaseNready(img->dev(), "P", "P23A");
	}
      else
	{

	  signalTimeSlip(img->dev(), "P", "P24");

	  continue;
	}

    } // for ( ; ; ; )
	  
#ifdef VERBOSE
  if ( 5 < verbose ) cerr << "Producer " << img->dev() 
		       << ":  returning." << endl << flush;
#endif
  return(NULL);
}
  

// Consumer thread.  Does stereo vision calculation
void *consume(void *arg)
{
  producerTimes tim[2];
  int dev = -1, otherDev;
  int numStep;
  if ( memTime ) numStep = NUMSTEP;
  else           numStep = NUMFILESTEP;
  float  epsilon;               // Small time step value
  float  maxStep = 0.0;         // Maximum allowed time step for velo calc
  float  dTball;                // Time since last triangulated ball

  // Load zone file
  char zonFile[MAX_CANON];
  sprintf(zonFile,"zone.tune");
  Zone *ZON = new Zone(zonFile);
#ifdef VERBOSE
  if ( 4 < verbose ) cout << *ZON << flush;
#endif

  BallSet *balls;
  if ( eqCam )
    balls = new EqCamBallS(singleCameraDist);
  else
    balls = new CamBallS;
  const int NUMPAST = 2;
  BallStats presentBall, checkBall, previousBall, pastBalls[NUMPAST];

  // initialize
  previousBall.hasOther = 0;
  int numPast = 0;
  for (int i=0; i<2; i++)
    {
      timerclear(&(tim[i].presLoad));
    }

  // Loop until broken
  for ( ; ; )
    {
      // Handle synchronization

      acquireNready  (nready.dev, "C", "C1");

      while ( 0 == nready.nready && nready.producer_done < numDev )
	{
	  waitNready  (nready.dev, "C", "C2");
	}
#ifdef VERBOSE
      if ( 5 < verbose )
	{
	  for (int j=0; j<numDev; j++)
	    cerr << "found[" << j << "].numThings = " << found[dev].numThings
		 << ".  ";
	  cerr << endl << flush;
	}
#endif

      if ( nready.producer_done >= numDev ) break;
      dev = nready.dev;

      nready.nready = MAX( nready.nready-1, 0 );
      if      ( 0 == dev ) otherDev = 1;
      else if ( 1 == dev ) otherDev = 0;
      else
	{
	  cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__ << " : "  
	       << "device wrong = " << dev << endl;
	  exit(1);
	}

      // Look for new objects, sort, and cull balls
      tim[dev]   = nready.timeSummary;

      acquireFound   (dev, "C", "C4");

#ifdef VERBOSE
      if ( 3 < verbose )
	{
	  printTimeResults(balls->procStart(), dev, tim[dev],
			   tim[otherDev], found[dev].dtime);
	}
#endif
      if ( 0 < found[dev].numThings )
	{

	  int numNewLight = 0;

	  // Find light balls
	  for (int j=0; j<found[dev].numThings; j++)
	    {
	      if ( balls->addBall(found[dev].thingP[j], dev, found[dev],
				  found[dev].pixP, tim[dev].procStart,
				  tim[dev].fps) )
		{
		  numNewLight++;
		  //  needed for checkSetup
		  if ( 1 < verbose || calibrate ) 
		    {
		      cout << tim[dev].deviceDigit << ": " << " L B Found @ "
			   << setprecision(6) << setw(11)
			   << (found[dev].presLoad - balls->procStart()) 
			   << " sec, " << *found[dev].thingP[j] << flush;
		    }
		}
	    }

	  // Sort balls by time to be able to find latest if any new ones
	  balls->sortBalls();
	  
	  // Skip if no new light balls
	  if ( !numNewLight ) goto noballs;

	  // Skip if not at least 2 balls and 2 devices
#ifdef VERBOSE
	  if ( 1 < verbose ) cout << numNewLight << " NEW LIGHT BALLS:\n" 
			      << *balls << flush;
#endif
	  if ( !( 2 < balls->numLightBalls() && 1 < numDev ) ) goto noballs;

	  
	  // Loop to move time of lone ball until vector rays match
	  float dTbias   = 0.;
	  float DT       = 2. / tim[dev].fps;
	  float epsT     = DT / 10.;
	  float errp = 0, err0 = 0, dTp = 0, dT0 = 0;
	  float errTol   = 1e-3;
	  int   triCount = 0;
	  int   atLimits = 0;
	triLoop:
	  triCount++;
	  if ( maxTriCount < triCount ) atLimits = 1;
	  if ( DT == std::abs(dTbias) && 2 > triCount ) atLimits = 1;

	  // Use new ball sighting only if increments time
	  if ( pickyTime ) presentBall = balls->latestBall(dTbias);
	  else
	    {
	      // Increment previousBall only if really have a new presentBall
	      checkBall = balls->latestBall(dTbias);
	      if ( checkBall.load-presentBall.load !=0.0 &&
		   !isnan(presentBall.err) )
		{
		  previousBall = pastBalls[0];
		}
	      presentBall = checkBall;
	    }

	  // Skip if ball has no interpolated match in other device
	  if ( !presentBall.hasOther ) goto noballs;

	  // Calculate velocity for two consequetive balls
	  PixelLoc3 position;
	  position = triangulate(presentBall);

	  if ( errTol < presentBall.err && !atLimits )
	    {
	      if ( 1 == triCount ) 
		{
		  err0   = presentBall.err;
		  dT0    = dTbias;
		  dTbias = epsT;
		}
	      else
		{
		  errp = err0;
		  dTp  = dT0;
		  err0   = presentBall.err;
		  dT0    = dTbias;
		  dTbias = fmax(fmin( dT0 -
				      err0 / (( errp -err0 ) / (dTp -dT0 )),
				      DT), -DT);
		}
	      goto triLoop;
	    } // if ( errTol < presentBall.err && !atLimits )

	  if ( 1 < verbose || calibrate ) 
	    cout << "CHOSEN: " << presentBall
		 << "BALL_POSITION(" << setprecision(6) << setw(9)
		 << (presentBall.load - balls->procStart())
		 << ")" << setprecision(1) << setw(1) << presentBall.device
		 << "="
		 << setprecision(6) << setw(11) << position
		 << " UNCERT="
		 << setprecision(6) << setw(10) << presentBall.Err
		 << " normUNCERT=" << presentBall.err
		 << " triCount="
		 << setprecision(2) << setw(2)  << triCount
		 << " dTbias="
		 << setprecision(2) << setw(3)  << dTbias
		 << endl << flush;

	  if ( previousBall.hasOther )
	    {
	      epsilon = 1.0 / (float)tim[dev].fps / 5.0;
	      maxStep = ( (float)numStep / (float)tim[dev].fps ) + epsilon;
	      dTball = presentBall.load - previousBall.load;
	      
	      // Reject slips
	      if ( !( epsilon < fabs(dTball) && maxStep > fabs(dTball) ) )
		{
#ifdef VERBOSE
		  if ( 1 < verbose )
		    {
		      cout << "BALL TIME REJECT: " << "epsilon="  
			   << setprecision(6) << setw(11)
			   << epsilon << ", dTball="
			   << setprecision(6) << setw(11)
			   << dTball
			   << ", maxStep="
			   << setprecision(6) << setw(11)
			   << maxStep << endl << flush;
		    }
#endif
		  goto reject;
		}
	      presentBall.velo = presentBall.pos - previousBall.pos;
	      presentBall.velo.scale( 1.0 / dTball );
	      presentBall.veloErr = (presentBall.err + previousBall.err)/dTball;
	      presentBall.VeloErr =  presentBall.Err + previousBall.Err;
	      presentBall.VeloErr.scale( 1.0 /dTball );
#ifdef VERBOSE
	      if ( 1 < verbose )
		cout << "BALL VELOCITY(" << setprecision(6) << setw(9)
		 << (presentBall.load - balls->procStart())
		 << ")" << setprecision(1) << setw(1) << presentBall.device
		 << "="
		 << setprecision(6) << setw(11) << presentBall.velo 
		 << " vEL UNCERTAINTY   ="  
		 << setprecision(6) << setw(11) << presentBall.veloErr 
		 << " VEL UNCERTAINTY   ="  
		 << setprecision(6) << setw(11) << presentBall.VeloErr 
		 << endl << flush;
#endif
	      
	      // Reject slow or fast balls
	      if ( presentBall.velo[1] > ZON->velThresh() ||
		   presentBall.velo[1] < ZON->velMax()      )
		{
		  cerr << "S" << flush;
#ifdef VERBOSE
		  if ( 1 < verbose ) cout << "BALL SPEED REJECT\n" << flush;
#endif
		  goto reject;
		}

	      // Reject uncertain triangulation
	      if ( isnan(presentBall.err) ||
		   presentBall.err > ZON->triangulationScalar()*ZON->ballDia() )
		{
		  cerr << "T" << flush;
#ifdef VERBOSE
		  if ( 1 < verbose ) cout << "BALL TRIANGULATION REJECT\n" 
					  << flush;
#endif
		  goto reject;
		}

	      // Calculate strike
	      PixelLoc3 avgVelo;
	      pixelLocf Pzone;
	      pixelLocf avgUncert;
	      PixelLoc3 avgVeloU;
	      Pzone  = avgZone(presentBall, previousBall, pastBalls, numPast,
			       ZON->gravity(), avgVelo, avgUncert, avgVeloU);
	      
#ifdef VERBOSE
	      float timeToZone =  fabs(presentBall.pos[1] / avgVelo[1]);
	      if ( 1 < verbose ) cout << "BALL timeToZone="
				      << setprecision(6) << setw(11)
				      <<timeToZone << endl << flush;
#endif

	      // Tune
	      Pzone.x += ZON->dxZone() - 
		ZON->dXdMphZone() * avgVelo[1] / ZON->speedDisplayScalar();
	      Pzone.y += ZON->dzZone() -
		ZON->dZdMphZone() * avgVelo[1] / ZON->speedDisplayScalar();
	      
	      // Announce
	      cerr << "C" << flush;
	      cout << ZON->call(Pzone.x, Pzone.y) << endl
		   << "(x, z)= ("
		   << setprecision(6) << setw(11)
		   << Pzone
		   << ") "
		   << setprecision(6) << setw(11)
		   << -avgVelo[1] / ZON->speedDisplayScalar() << " "
		   << setprecision(6) << setw(11)
		   << speedDisplayUnits
		   << "; (t, y)= ("
		   << setprecision(6) << setw(11)
		   << (presentBall.load - balls->procStart()) << ", "
		   << setprecision(6) << setw(11)
		   << presentBall.pos[1]
		   << ");   UNCERT="
		   << setprecision(6) << setw(11)
		   << avgUncert << ";"
		   << endl << flush;
	      if ( 1 < verbose ) // verbose needed for checkFile
		cout << "BALL ZONE= "
		     << setprecision(6) << setw(11)
		     << (presentBall.load - balls->procStart()) << " "
		     << setprecision(6) << setw(11)
		     << Pzone << " "
		     << setprecision(6) << setw(11)
		     << presentBall.err << " " 
		     << setprecision(6) << setw(11)
		     << avgVelo << " "
		     << position[1] << endl << flush;
	    }  // if ( previousBall.hasOther ) 
	reject:
#ifdef VERBOSE
	  if ( 1 < verbose ) 
	    {
	      cout << "BALL UNCERTAINTY(" 
		   << setprecision(6) << setw(11)
		   << (presentBall.load - balls->procStart()) << ")="
		   << setprecision(6) << setw(11)
		   << presentBall.err << endl << flush;
	      cout << "BALL VELOCITY         ="
		   << setprecision(6) << setw(11)
		   << presentBall.velo << endl << flush;
	      cout << "BALL PREV POSITION(" 
		   << setprecision(6) << setw(11)
		   << (previousBall.load - balls->procStart()) << ")"
		   << setprecision(6) << setw(11)
		   << previousBall.device << "="
		   << setprecision(6) << setw(11)
		   << previousBall.pos << endl << flush;
	      cout << "BALL PREV VELOCITY="
		   << setprecision(6) << setw(11)
		   << previousBall.velo << endl << flush;

	      // Isolate swapped wires
	      if ( presentBall.velo[1] > 0 &&
		   presentBall.err > ZON->triangulationScalar()*ZON->ballDia()&&
		   previousBall.velo[1] > 0  &&
		   previousBall.err > ZON->triangulationScalar()*ZON->ballDia())
		cerr << "WARNING(ump):  POSSIBLY SWAPPED CAMERA WIRES\n";
	    }
#endif

	  // Save good balls only
	  if ( 0 < sqnorm(presentBall.pos) && !isnan(presentBall.err) )
	    {
	      for (int i = NUMPAST; i > 1; i--) pastBalls[i-1] = pastBalls[i-2];
	      numPast = MIN(numPast+1, NUMPAST);
	      pastBalls[0] = presentBall;
	      sortPastBalls(pastBalls, numPast, presentBall.load, maxStep*2);
#ifdef VERBOSE
	      if ( 5 < verbose )
		{
		  cerr << "                              " << flush;
		  if ( 1 == dev ) cerr << "              " << flush;
		  cerr << "after sortPastBalls C6\n" << flush;
		}
#endif
	    }


	  if ( pickyTime ) previousBall = pastBalls[0];
	} // End of new balls
    noballs:

      // Check for memory leaks
#ifdef HAVE_SBRK
#ifdef VERBOSE
      if ( 5 < verbose ) checkMem("WARNING(ump): end of iteration");
#endif
#endif

      releaseFound(dev, "C", "C7");

      // WATCH OUT FOR THIS.  ALWAYS NEED TO SIGNAL IF nready HAS CHANGED
      // IF GET HANGING PROCESSES, use -v6 option and run.
      // LOOK FOR more instances of "Wait for nready" than "Signaled nready"
      // as indication of the problem.
      if ( 0 == nready.nready )
	{

	  signalNready(dev, "C", "C8");

	}

      releaseNready(dev, "C", "C9");

    }  // for ( ; ; ; )
  nready.nready = 0;  // Force completion
  signalNready (dev, "C", "C11");
  releaseNready(dev, "C", "C11");
 
  zap(balls);
  zap(ZON);
#ifdef VERBOSE
  if ( 5 < verbose ) 
    {
      cerr << "                              " << flush;
      if ( 1 == dev ) cerr << "              " << flush;
      cerr << "Consumer " << dev << ":  returning." << endl << flush;
    }
#endif
  return(NULL);
}

// Signal handler
void *myhand_thread(void *arg)
{
  int      signo;
  struct   sched_param param;
  int      policy;
  char     susp_message[] = "\nContinue? [ynwph] (h for help) >";
  char     rdy_message[]  = "interrupt(ctrl-c) OR quit(ctrl-|) >";
  char     susp_answer[2];
  int      restartPipe = 0;
  int      restarted   = 0;

  // Show status
  Pthread_getschedparam(pthread_self(), &policy, &param);

  // Wait for signals
  write(STDERR_FILENO, rdy_message, strlen(rdy_message));
  for ( ; ; )
    {
      sigwait(&intmask, &signo);
      acquireIntLock(0, "H", "H1");

      switch (signo)
	{
	case SIGINT:
	askagain:
	fflush(stderr);
	fflush(stdout);
	sleep(1);  // Hack to fix a race condition.
	write(STDERR_FILENO, susp_message, strlen(susp_message));
	read (STDIN_FILENO,  &susp_answer, 2);
	switch ( susp_answer[0] )
	  {
	  case 'h' : case 'H':
	    cerr << "OPTIONS(ump):\n"
		 << "    y,Y = continue\n"
		 << "n,N,q,Q = quit\n"
		 << "    w,W = write snapshot\n"
		 << "    p,P = pause\n";
	    goto askagain;
	  case 'p' : case 'P':
	    if ( !memTime && !realTime )
	      {
		cerr << "You are already paused for file input.\n";
		goto askagain;
	      }
	    cerr << "Pausing...\n";
	    if ( memTime )
	      {
		if ( restarted )
		  stopChildren();
		else
		  kill (ppid, SIGUSR1);
	      }
	    else
	      {
		for ( int i=0; i<numDev; i++ )
		  {
		    if ( 0 > kill (childpid[i], SIGSTOP) )
		      {
			cerr << "WARNING(ump): "<< __FILE__ << " : " 
			     << __LINE__ << " : "  << "stopping streamer : ";
			perror("");
		      }
		    else
		      {
			cerr << "MESSAGE(ump):  paused child pid="
			     << childpid[i] << endl;
		      }
		  }
	      }
	    restartPipe = 1;
	    goto askagain;
	  case 'W' : case 'w':
	    cerr << "asking for a snapshot.\n";
	    if ( !memTime && !realTime )
	      {
		cerr << "must be mem mapping for that.\n";
		goto askagain;
	      }
	    int_lock.snap[0] = 1;
	    int_lock.snap[1] = 1;
	  case 'y' : case 'Y':
	    if ( restartPipe )
	      {
		cerr << "Restarting pipe...\n";
		if ( memTime )
		  {
		    int *ifd[2]   = {new int[2], new int[2]};
		    int *ofd[2]   = {new int[2], new int[2]};
		    for (int i=0; i<numDev; i++)
		      {
			smptr[i] = 
			  pipeStreamerFork(((Image**)arg)[i]->width(),
					   ((Image**)arg)[i]->height(),
					   ((Image**)arg)[i]->device(),
					   ((Image**)arg)[i]->fps(), absframes,
					   deadband, &smid[i],
					   &childpid[i], ifd[i], ofd[i], 
					   ((Image**)arg)[i]->maxEdgePix(),
					   smptr[i], ((Image**)arg)[i]->sub(),
					   loadTest, loadLevel);
			((Image**)arg)[i]->reassignSmptr(smptr[i]);
			((Image**)arg)[i]->reassignIfd(ifd[i][0]);
			((Image**)arg)[i]->reassignOfd(ofd[i][1]);
		      }
		    sleep(2);  // Wait for both streamers to get going
		    for (int i=0; i<numDev; i++)
		      {
			((Image**)arg)[i]->resetTimers();
		      }
		    restartPipe = 0;
		  }
		else if ( realTime )
		  {
		    for (int i=0; i<numDev; i++)
		      {
			if ( 0 > kill (childpid[i], SIGCONT) )
			  {
			    cerr << "ERROR(ump):  "<< __FILE__ << " : " 
				 << __LINE__ << " : " 
				 << "restartining streamer : ";
			    perror("");
			    exit(1);
			  }
			cerr << "MESSAGE(ump): " << getpid() 
			     << " restarted child " << childpid[i] << endl;
			((Image**)arg)[i]->resetTimers();
		      }
		    restartPipe = 0;
		  }
		restarted = 1;
	      }
	    cerr << "Continuing...\n";
	    if ( realTime )
	      cerr << "MESSAGE(ump):  oops alarm messages normal "
		   << "result of long pause\n";
	    Pthread_cond_broadcast(&int_lock.cond);
	    goto keepgoing;
	  case 'n' : case 'N': case 'q' : case 'Q':
	    break;
	  default:
	    goto askagain;
	  }

	case SIGQUIT:
	  fflush(stderr);
	  fflush(stdout);
	  sleep(1);  // Hack to fix a race condition.
	  
	default:
	  cerr << "Quitting...\n";
	  int_lock.shutdown = 1;
	  Pthread_cond_broadcast(&int_lock.cond);

	  releaseIntLock(0, "H", "H2");

	  // Kill wait if needed

	  acquireNready (0, "H", "H3");

	  nready.producer_done = numDev;
	  timeSlip.lastDev = -1;

	  signalTimeSlip(0, "H", "H4");
	  signalNready  (0, "H", "H5");

	  releaseNready (0, "H", "H6");

	  return(NULL);
	}
      
    keepgoing:
      write(STDERR_FILENO, rdy_message, strlen(rdy_message));
      releaseIntLock(0, "H", "H7");
    }
  
  return(NULL);
}

