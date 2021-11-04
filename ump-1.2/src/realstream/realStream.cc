// Real time interface to xawtv streamer routines 
// 	$Id: realStream.cc,v 1.1 2007/01/13 22:20:20 davegutz Exp davegutz $	 

// Includes section
using namespace std;
#include <common.h>
#include "config.h"
#include <iostream>    // cerr
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <X11/Intrinsic.h>
#include <sys/param.h> // built-in constants and MIN/MAX
#define   SIGN(A)         ((A) < (0) ? (-1): (1))
extern "C"
{
 #include "grab.h"
 #include "grab-v4l-stat.h"
 #include "Cwrappers.h"
 #include "makeargv.h"
 #include "commands.h"
 #include "capturey.h"
}
#include <string>      // string classes
#include <string.h>    // strings

extern int calibrate;

static int signaled;
static int                      MAXMASK  = 20;
static int                      MASKSIZE =  7;
static int                      NUMARGS  =  4;
static int                      quiet    =  1;  // verbosity           
struct GRAB_FORMAT {
    char *name;
    int  bt848;
    int  mul,div;
    int  can_stdout;
    int  can_singlefile;
    int  can_multiplefile;
};
static struct GRAB_FORMAT formats[] = {
    // file formats 
    { "ppm",  VIDEO_RGB24,     3,1,  0,0,1 },
    { "pgm",  VIDEO_GRAY,      1,1,  0,0,1 },
    { "jpeg", VIDEO_RGB24,     3,1,  0,0,1 },

    // video 
    { "avi15",VIDEO_RGB15_LE,  2,1,  0,1,0 },
    { "avi24",VIDEO_BGR24,     3,1,  0,1,0 },
    { "mjpeg",VIDEO_MJPEG,     0,1,  0,1,0 },

    // raw data 
    { "rgb",  VIDEO_RGB24,     3,1,  1,1,1 },
    { "gray", VIDEO_GRAY,      1,1,  1,1,1 },
    { "422",  VIDEO_YUV422,    2,1,  1,1,1 },
    { "422p", VIDEO_YUV422P,   2,1,  1,1,1 },
    { "420p", VIDEO_YUV420P,   3,2,  1,1,1 },

    // end of table 
    { NULL,   0,               0,0,  0 },
};

// Mask structure and functions 
struct window
{
  int Sx;
  int Sy;
  int Dx;
  int DDx;
  int Dy;
  int DDy;
  int add;
};
static int winVal(int x, int y, struct window W)
{
  if ( x    >= W.Sx + (float)(y - W.Sy) / (float)W.Dy * (float)W.DDx
       && x <= W.Sx + (float)(y - W.Sy) / (float)W.Dy * (float)W.DDx + W.Dx
       && y >= W.Sy + (float)(x - W.Sx) / (float)W.Dx * (float)W.DDy 
       && y <= W.Sy + (float)(x - W.Sx) / (float)W.Dx * (float)W.DDy + W.Dy   )
    return W.add;
  else return 0;
}
// Concurrency data passing structure 
struct reading_t
{
  long startV;
  long numV;
  long loadSec;
  long loadUsec;
  long ploadSec;
  long ploadUsec;
};

// Handle loss of communication 
static void brokenPipe(int signal)
{
  char *msg = "MESSAGE(realStream):  broken pipe\n";
  if ( !quiet ) write(STDERR_FILENO, msg, strlen(msg));
  write(STDERR_FILENO, msg, strlen(msg));
  signaled = 1;
}

// Handle instructions from caller 
static void interrupted(int signal)
{
  char *msg = "MESSAGE(realStream):  interrupted\n";
  write(STDERR_FILENO, msg, strlen(msg));
  signaled = 1;
}

// Substring substitution.
static void gsub(string *expr, const string target, const string replace)
{
  unsigned int j=0;
  while ( (j=expr->find(target)) < expr->length() )
    {
      expr->replace(j, target.length(), replace);
    }
}

// Make .set file name
static string makeSetFileName(char *filename)
{
  string setfilename(filename);
  gsub(&setfilename, ".raw", ".set");
  return setfilename;
}

// Make .tim file name
static string makeTimFileName(char *filename)
{
  string timFilename(filename);
  gsub(&timFilename, ".raw", ".tim");
  return timFilename;
}

// Provide file handle 'talk' to request data load to 'buffers' from 
// spawned stream process 
int *realStreamerFork(int *width, int *height, char *device, int grab_fps,
		      int absframes, int deadband, int *smid, int *childpid,
		      int *talk, int maxedgepix, int *smptr, int sub,
		      int loadTest, int loadLevel, int *format, char *input,
		      char *tvnorm)
{
  int status, waitpid;   // pid wait status 
  struct window inMask[MAXMASK]; // Masks 
  struct reading_t reading; // Info 
  int   numInMask;       // Number of masks 
  int  *maski;           // Index of used pixels 
  int  *maskj;           // Index of used pixels 
  int  *mi;              // Pointer to mask 
  int  *mj;              // Pointer to mask 
  int   numMask      = 0;// Number of mask indeces 
  int   maskVal;         // Local status of mask 
  long  args[NUMARGS];   // read/write pipe arguments
  long  pasttimeSec  = 0;// past image time stamp, sec 
  long  pasttimeUsec = 0;// past image time stamp, sec 
  int   i;               // local index 
  int   j;               // local index 
  int   k;               // local index 
  int   im;              // local index 
  int  *smptrT;          // temporary pointer 
  long  argspast;        // last image info 
  int   delta;           // change since last image 
  int   deltaP;          // auxilliary calculation variable 
  int   iread;           // to count bytes returned by reads
  long  readargs[BUFSIZ];// read/write pipe arguments
  int   startV = 0;      // memory start point
  int   numV   = 0;      // number of points in shm
  int   rdgbuflength;    // length of reading in shm
  int   index;           // local index
  int   bufsize;         // reading buffer size
  int   linelength = 0;  // video width * depth
  int   wsync;           // synchronization    
  int   s[2];            // pipe handle        
  int   BUFCOUNT   = 8;  // number of frame-grabber buffers
  unsigned char* grab_buffers;   // frame-grabber buffer
  unsigned char *buffersptr;     // present image start
  unsigned char *bufferspastptr; // previous image start

  // Synchronization
  struct timeval  grab_start;
  struct timeval  tv;           
  static int      synctime = 500;
  int   grab_bfirst    = 0;
  int   grab_blast     = 0;
  int   grab_bufcount;
  int   grab_bufsize;
  int   grab_lastsec   = 0;
  int   grab_lastsync  = 0;
  int   grab_absqueued = 0;
  int   grab_secframes = 0;
  int   grab_secqueued = 0;
  int   grab_missed    = 0;
  int   timediff;
  int   size;
  int   shmid    = -1;
  int   syncpid; 

  // initialize
  rdgbuflength   = 4*maxedgepix;
  reading.startV =    (long)(0);
  reading.numV   =    (long)(0);
  argspast       = BUFCOUNT - 1;
      
  // Create shared memory segment for streamer to link to.
  if ( NULL == smptr )
    {
      *smid = Shmget(ftok(device, 1), 4*rdgbuflength*sizeof(int), 
		  SHM_R | SHM_W | SHM_R>>3 | SHM_R>>6 | IPC_CREAT | IPC_EXCL );
      if ( -1 == *smid ) cerr << "continuing after shmget failure...\n";
    }
  
  // Set format
  for ( i = 0; formats[i].name != NULL; i++ )
    if ( 0 == strcasecmp(formats[i].name, "gray") ) 
      {
	*format = i;
	break;
      }
  if ( formats[i].name == NULL )
    {
      cerr << "ERROR(realstream): " << __FILE__ << " : " << __LINE__
	   << " unknown format " << optarg <<  " (available:";
      for ( i = 0; formats[i].name != NULL; i++ )
	cerr << " " << formats[i].name;
      cerr << ")\n";
      exit(1);
    }

  // Open device
  setStatics();
  setStatics2();
  grabber_open(device, 0, 0, 0, 0, 0);
  if ( grabber->grab_setparams == NULL || grabber->grab_capture == NULL)
    {
      cerr << "ERROR(realstream): " << __FILE__ << " : " << __LINE__
	   << grabber->name << "\ncapture not supported\n";
      exit(1);
    }
  
  // Modify settings      
  do_va_cmd(2, "setinput", input);
  do_va_cmd(2, "setnorm", tvnorm);
  
  // Set capture parameters
  grabber_setparams(formats[*format].bt848, width, height, &linelength, 0);
  
  // Start writer
  if (-1 == socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, s)) {
    cerr << "ERROR(realstream): " << __FILE__ << " : " << __LINE__ << " ";
    perror("socketpair");
    exit(1);
  }
  switch(*childpid = fork())
    {
    case -1:
      perror("fork");
      close(s[0]);
      close(s[1]);
      break;
    case 0:  // child
      Setpgid(getpid(), getpid()); // divorce from parent sigint
      signal(SIGINT,  SIG_IGN);
      signal(SIGPIPE, brokenPipe);
      close(s[1]);
      *talk = s[0];
      smptr = (int *) Shmat(*smid, NULL, 0);

      // buffer initialization
      if ( absframes < BUFCOUNT ) BUFCOUNT = absframes+2;
      bufsize = *width * *height * formats[*format].mul / formats[*format].div;
      if ( 0 == bufsize ) bufsize = *width * *height * 3;
      while ( BUFCOUNT > 0 )
	{
	  shmid = Shmget(IPC_PRIVATE, bufsize*BUFCOUNT, IPC_CREAT | 0700);
	  if ( -1 != shmid ) break;
	}
      if ( 0 == BUFCOUNT ) 
	{
	  perror("shmget");
	  return NULL;
	}
      grab_buffers = (unsigned char *) Shmat(shmid, 0, 0);
      if ( (void *) -1 == grab_buffers )
	{
	  cerr << "ERROR(realstream): " << __FILE__ << " : " << __LINE__
	       << " buffer not available\n";
	  exit(1);
	}
      Shmctl(shmid, IPC_RMID, 0);
      gettimeofday(&grab_start,NULL);
      grab_bufcount  = BUFCOUNT;
      grab_bufsize   = bufsize;
  
      // load mask, block on input now
      iread = read(*talk, &numInMask, sizeof(int));
      if ( -1 == iread
	   && errno != EAGAIN )
	{
	  cerr << "ERROR(realstream): " << __FILE__ << " : " << __LINE__<< " ";
	  perror("writer: cmd socket");
	  exit(1);
	}
      if ( bufsize <= numInMask*MASKSIZE || numInMask >= MAXMASK )
	{
	  cerr << "ERROR(realstream): " << __FILE__ << " : " << __LINE__
	       << " mask too large.\n";
	  exit(1);
	}
      maski = (int *) malloc(bufsize*sizeof(int));
      maskj = (int *) malloc(bufsize*sizeof(int));
      // Read the mask
      if ( 0 < numInMask )
	{
	  for (i=0; i<numInMask; i++)
	    {
	      int index = i*MASKSIZE;
	      inMask[i].Sx = smptr[index    ];
	      inMask[i].Sy = smptr[index + 1];
	      inMask[i].Dx = smptr[index + 2];
	      inMask[i].DDx= smptr[index + 3];
	      inMask[i].Dy = smptr[index + 4];
	      inMask[i].DDy= smptr[index + 5];
	      inMask[i].add= smptr[index + 6];
	    }
	  for (j=0; j<*height; j++) for (i=0; i<*width; i++)
	    {
	      maskVal  = 0;
	      if ( ( 0 == j%sub ) && ( 0 == i%sub ) )
		{
		  for (k=0; k<numInMask; k++)
		    maskVal += winVal(i, j, inMask[k]);
		}
	      if  ( 0 < maskVal )
		{
		  maski[numMask]   = i;
		  maskj[numMask++] = j;
		}
	    }
	}
      else
	{
	  for (j=0; j<*height; j++) for (i=0; i<*width; i++)
	    {
	      if ( ( 0 == j%sub ) && ( 0 == i%sub ) )
		{
		  maski[numMask]   = i;
		  maskj[numMask++] = j;
		}
	    }
	}

      // Setup load test
      if ( loadTest )
	{
	  startV = reading.startV;
	  smptrT = &smptr[startV];
	  numV = 0;
	  if ( 1 == loadLevel )
	    k = (int)sqrt((float)maxedgepix);
	  else
	    k = 0;
	  for ( i=0; i<k; i+=sub )
	    {
	      for ( j=0; j<k; j+=sub)
		{
		  numV++;
		  if ( numV > maxedgepix )
		    {
		      goto loaddone;
		    }
		  *smptrT++ = (*height-k)/2 + j;
		  *smptrT++ = (*width -k)/2 + i;
		  *smptrT++ = 100;
		  *smptrT++ = 227;
		}
	    }
	loaddone:
	  // 4 less for corner pix added in Image
	  numV = MAX(MIN(numV, maxedgepix/sub - 5), 2);
	}
      else  // run forever
	{
	  absframes = 1000000;
	}

      // setup non-blocking sync
      fcntl(*talk, F_SETFL, O_RDWR|O_NONBLOCK);
      syncpid = grab_syncer(&wsync, quiet);
      if ( !quiet ) fprintf(stderr,
	     "MESSAGE(realStream): started syncer pid=%d for device %s\n",
	      syncpid, device);

      // loop until signaled or limit exceeded
      signal(SIGQUIT, interrupted);
      for ( ; grab_absqueued < absframes && !signaled ;  )
	{
	  // get next frame
	  grabber_capture(grab_buffers+grab_bufsize*grab_bfirst, 0, 0, &size);

	  // check time
	  gettimeofday(&tv,NULL);
	  timediff  = (tv.tv_sec  - grab_start.tv_sec)  * 1000;
	  timediff += (tv.tv_usec - grab_start.tv_usec) / 1000;
	  if (timediff > grab_lastsync + synctime)
	    {
	      // sync
	      grab_lastsync = timediff - timediff % synctime;
	      
	      if ( sizeof(grab_bfirst) != 
		   write(wsync, &grab_bfirst, sizeof(grab_bfirst)) )
		{
		  cerr << "ERROR(realstream): " << __FILE__ << " : " 
		       << __LINE__<< " ";
		  perror("grabber: write socket");
		  exit(1);
		}
	    }
	  if (timediff > grab_lastsec+1000)
	    {
	      // print statistics
	      grab_missed += (grab_fps - grab_secqueued);
	      if ( !quiet && grab_secframes )
		fprintf(stderr, " %2d/%2d (%d)\n",
			grab_secqueued, grab_secframes, -grab_missed);
	      grab_lastsec = timediff - timediff % 1000;
	      grab_secqueued = 0;
	      grab_secframes = 0;
	    }
	  
	  // check for free buffers
	  switch( read(*talk, args, sizeof(long)) )
	    {
	    case -1:
	      if (errno != EAGAIN)
		{
		  cerr << "ERROR(realstream): " << __FILE__ << " : " 
		       << __LINE__<< " ";
		  perror("grabber: read socket");
		  exit(1);
		}
	      break;
	    case 0:
	      // nothing
	      break;
	    default:
	      grab_blast++;
	      if ( grab_blast == grab_bufcount )
		grab_blast = 0;
	      break;
	    }
	  
	  grab_secframes++;
	  if ( (grab_bfirst+2) % grab_bufcount == grab_blast )
	    {
	      // no buffer free
	      if ( !quiet )
		fprintf(stderr,"o");
	      goto skipgrab;
	    }
	  grab_secqueued++;
	  grab_absqueued++;
	  grab_bfirst++;
	  if ( grab_bfirst == grab_bufcount )
	    grab_bfirst = 0;
	  if ( !quiet )
	    fprintf(stderr,"+");
	  
	  // queue buffer
	  args[0]        = (long) grab_bfirst;
	  args[1]        = (long) size;
	  args[2]        = tv.tv_sec;
	  args[3]        = tv.tv_usec;
	  buffersptr     = grab_buffers + args[0]  * bufsize;
	  if ( 8 > iread )
	    bufferspastptr = grab_buffers + argspast * bufsize;
	  else
	    bufferspastptr = grab_buffers + readargs[iread-8] * bufsize;
	  argspast = args[0];
	  
	  // write out
	  startV = reading.startV;
	  smptrT = &smptr[startV];
	  if ( !loadTest ) numV   = 0;
	  for ( im=0, mi=maski, mj=maskj; im<numMask; im++, mi++, mj++ )
	    {
	      index    = (*mj)* *width + (*mi);
	      delta    = (int)(buffersptr[index])-(int)(bufferspastptr[index]);
	      deltaP   = abs(delta) - deadband;
	      if ( deltaP > 0 && !loadTest )
		{
		  *smptrT++ = *mj;
		  *smptrT++ = *mi;
		  *smptrT++ = (deltaP + deadband) * SIGN(delta);
		  *smptrT++ = (int) buffersptr[index];
		  numV++;
		}
	      if ( 0 == loadLevel ) numV = 0;
	      
	      // ignore data if too many pixels changed
	      if ( numV >= maxedgepix )
		{
		  fprintf(stderr, "*");
		  numV = 0;
		  goto overflow;
		}
	    }
	overflow:
	  
	  // write buffer information
	  reading.startV    = (long)startV;
	  reading.numV      = (long)numV;
	  reading.loadSec   = args[2];         // time sec
	  reading.loadUsec  = args[3];         // time usec
	  reading.ploadSec  = pasttimeSec;     // prev time sec
	  reading.ploadUsec = pasttimeUsec;    // prev time usec
	  
	  write(*talk, &reading, sizeof(struct reading_t));
	  
	  pasttimeSec  = args[2];
	  pasttimeUsec = args[3];
	  
	  // reset synchro buffers
	  reading.numV = (long)(0);
	  if ( 0 == iread )
	    fprintf(stderr, ">");
	  if ( !quiet )
	    fprintf(stderr,"-");
	  
	skipgrab:
	  continue;
	}       //for ( ; grab_absqueued < absframes && !signaled ;  )
      free(maski);
      free(maskj);
      grabber->grab_close();
      setStatics();
      setStatics2();
      shutdown(*talk, 1);
      Close(wsync);
      if ( 0 > kill (syncpid, SIGQUIT) )
	{
	  perror ("WARNING(ump):  stopping syncer");
	}
      while( syncpid != (waitpid = wait(&status)) )
	if ( (-1 == waitpid) && (EINTR != errno) )
	  {
	    perror("WARNING(realStream):  wait");
	    break;
	  }
      if ( !quiet )
	{
	  fprintf(stderr, "MESSAGE(realSteam): stopped syncer pid=%d\n",
		  waitpid);
	}
      fprintf(stderr,
	  "MESSAGE(realStream): exiting frame-grabber pid=%d for device %s\n",
	   getpid(), device);
      exit(0);
      break;
    default: // parent
      Close(s[0]);
      *talk = s[1];
      fcntl(*talk, F_SETFL, O_RDWR);
      signal(SIGPIPE, SIG_IGN);
      nice(5);
      smptr  = (int *) Shmat(*smid, NULL, 0);
      shmctl (*smid, IPC_RMID, NULL);
      break;
    }
  return smptr;
}

// Provide a file handle with data.  Connect control to ifd[0] and ofd[1].
// Return pointer to memory.
int *pipeStreamerFork(int width, int height, char *device, int fps,
		      int absframes, int deadband, int *smid, int *childpid,
		      int ifd[], int ofd[], const int maxedgepix, int *smptr,
		      int sub, int loadTest, int loadLevel)
{
  char   delim[] = " \t"; // Local string set delimiter
  char **streamerargv;    // Local argument string     
  int    numtokens;
  char   input_line[MAX_CANON];
  int    rdgbuflength = 4*maxedgepix;

  // Create shared memory segment for streamer to link to.
  if ( NULL == smptr )
    {
      *smid= Shmget(ftok(device, 1), 4*rdgbuflength*sizeof(int), 
		    SHM_R | SHM_W | SHM_R>>3 | SHM_R>>6
		    | IPC_CREAT | IPC_EXCL );
      if ( -1 == *smid )
	{ 
	  cerr << "ERROR(realstream): " << __FILE__ << " : " << __LINE__
	       << " quitting\n";
	  exit(1);
	}
      smptr = (int *) Shmat(*smid, NULL, 0);
    }

  // Process 1
  signal(SIGPIPE, brokenPipe);  // This prevents zombie
  Pipe(ifd);
  Pipe(ofd);
  *childpid = Fork();
  if ( 0 == *childpid )
    {      // Child

      // Set group to divorce from parent SIGINT and allow restart.
      Setpgid(getpid(), getpid());
      Dup2(ifd[1], STDOUT_FILENO);
      Dup2(ofd[0], STDIN_FILENO);
      Close(ifd[0]);
      Close(ifd[1]);
      Close(ofd[0]);
      Close(ofd[1]);
          if ( loadTest )
	sprintf(input_line, "streamery -q -s%dx%d -c %s -n NTSC -t%d -r%d -i \
Composite1 -V%d  -f gray -D%d -S%d -l%d",
		width, height, device, absframes, fps, maxedgepix, deadband,
		sub, loadLevel);
      else
	sprintf(input_line, "streamery -q -s%dx%d -c %s -n NTSC -t%d -r%d -i \
Composite1 -V%d  -f gray -D%d -S%d",
	      width, height, device, absframes, fps, maxedgepix, deadband,sub);
      if ( 0 > (numtokens = makeargv(input_line, delim, &streamerargv)) )
	{
	  cerr << "ERROR(realstream): " << __FILE__ << " : " << __LINE__
	       << "  could not construct argument array for " << input_line
	       << endl;
	  exit (1);
	}
      cerr << input_line << endl;
      // Start streamer function.
      Execvp(streamerargv[0], &streamerargv[0]);
      exit (1);
    }     // end child
  else
    {     // parent
      Close(ifd[1]);
      Close(ofd[0]);
    }

  if ( !smptr )
    {
      cerr << "ERROR(realstream): " << __FILE__ << " : " << __LINE__
	   << " unable to spawn pipe streamer\n";
      exit(1);
    }
  return smptr;
} // pipeStreamerFork

// Provide a child pid from streamer.
int writeStreamerFork(char *device_digit, char *outFile, int *width,
		      int *height, char *device, int *fps, int *absframes)
{
  char   setFile[NAME_MAX];    // Local file name
  char   timFile[NAME_MAX];    // Local file name
  int    childpid;        // Local fork name
  char   delim[] = " \t"; // Local string set delimiter
  char **streamerargv;    // Local argument string

  strcpy(setFile, makeSetFileName(outFile).data());
  strcpy(timFile, makeTimFileName(outFile).data());
  // Check for read-only files
  int quitting = 0;
  FILE *fp;
  if ( NULL == (fp = fopen(outFile, "w")) )
    {
      cerr << "ERROR(ump):  "<< __FILE__ << " : " << __LINE__
	   << " : protected " << setFile << ".  Quitting"
	   << endl << flush;
      quitting = 1;
	}
  else
    {
      fclose(fp);
    }
  if ( NULL == (fp = fopen(setFile, "w")) )
    {
      cerr << "ERROR(ump):  "<< __FILE__ << " : " << __LINE__
	   << " : protected " << setFile << ".  Quitting"
	   << endl << flush;
      quitting = 1;
    }
  else
    {
      fclose(fp);
    }
  if ( NULL == (fp = fopen(timFile, "w")) )
    {
      cerr << "ERROR(ump):  "<< __FILE__ << " : " << __LINE__
	   << " : protected " << timFile << ".  Quitting"
	   << endl << flush;
      quitting = 1;
    }
  else
    {
      fclose(fp);
    }
  if ( quitting ) exit(1);

  childpid = Fork();
  if ( 0 == childpid )
    {
      // Streamer child
      // Remove files to correct system clash
      char cmd[MAX_CANON];
      sprintf(cmd, "rm -f ./%s ./%s ./%s", outFile, setFile, timFile);
      if ( 0 != Execute(cmd, 0) )
	{
	  cerr << "ERROR(ump):  "<< __FILE__ << " : " << __LINE__
	       << " : cmd : " << cmd << " failed.  Quitting"
	       << endl << flush;
	  exit(1);
	}
      if ( NULL == (fp = fopen(setFile, "w")) )
	{
	  cerr << "WARNING(ump):  "<< __FILE__ << " : " << __LINE__
	       << " : cannot open " << setFile << " for writing.  Quitting"
	       << endl << flush;
	  exit(1);
	}
      else
	{
	  fprintf(fp, "%d %d\n %d %d %c\n", 
		  *width, *height, *fps, *absframes, *device_digit);
	  fclose(fp);
	}
      // execvp argument list
      char input_line[MAX_CANON];
      int numtokens;
      if ( calibrate )
	sprintf(input_line, "streamery -q -c %s -C -s%dx%d -n NTSC -t%d -r%d -i Composite1 -f gray -o%s",
		device, *width, *height, *absframes, *fps, outFile);
      else
	sprintf(input_line, "streamery -q -c %s -s%dx%d -n NTSC -t%d -r%d -i Composite1 -f gray -o%s",
		device, *width, *height, *absframes, *fps, outFile);
      if ( 0 > (numtokens = makeargv(input_line, delim, &streamerargv)) )
	{
	  cerr << "ERROR(ump): " << __FILE__ << " : " << __LINE__ << " : "  
	       << "could not construct argument array for " 
	       << input_line << endl;
	  exit (1);
	}
      for(int i=0; i<numtokens; i++)
	cout << streamerargv[i] << " ";
      cout << endl << flush;
    
      // Start streamer function.
      Execvp(streamerargv[0], &streamerargv[0]);
      flush(cout);
      exit (0);
    }

  if( -1 == childpid )
    {
      cerr << "ERROR(ump): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "unable to stream to file\n";
      exit(1);
    }
  return(childpid);
}  // writeStreamerFork

// Provide a file handle with data by opening existing file for reading.
int *readStreamerFork(char *device_digit, char *inFile, int *width,
			int *height, int *fps, int *absframes, int limitRead)
{
  FILE  *fp;            // Local file pointer
  int   *ifp;           // File handle return
  ifp = new int[2];

  // Open .tim file
  string timfilename = makeTimFileName(inFile);
  if ( 0 > (ifp[0] = open(timfilename.data(), O_RDONLY)) )
    {
      cerr << "ERROR(ump): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "cannot open " << timfilename << " for reading\n";
      exit(1);
    }

  // Read information from .set file
  string setfilename = makeSetFileName(inFile);
  if ( NULL == (fp = fopen(setfilename.data(), "r")) )
    {
      cerr << "ERROR(ump): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "cannot open " << setfilename<< " for reading\n";
      close(ifp[0]);
      exit(1);
    }
  int num;
  int readAbsFrames;
  if ( 5 != (num=fscanf(fp, "%d %d  %d %d %c",
		   width, height, fps, &readAbsFrames, device_digit)) )
    {
      cerr << "ERROR(ump): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "problem reading file " << setfilename
	   << ".  Should be 5 fields width, height, fps, absframes, device_digit\n";
      fclose(fp);
      close(ifp[0]);
      exit(1);
    }
  if ( limitRead ) *absframes = min(readAbsFrames, *absframes);
  else             *absframes = readAbsFrames;
  fclose(fp);

  // Open data file
  if ( !(ifp[1] = open(inFile, O_RDONLY)) )
    {
      cerr << "ERROR(ump): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "cannot open " << inFile << " for reading\n";
      close(ifp[0]);
      exit(1);
    }

  return(ifp);
}  // readStreamerFork

