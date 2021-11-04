/*
 *   (c) 1997-99 Gerd Knorr <kraxel@goldbach.in-berlin.de>
     Modified 10-13-01 by Dave Gutz <davegutz@alum.mit.edu>
     Application to stream video to file or to a shared memory.
 *
 */

/* Global Variables */
unsigned char *fileBuff;
long *timeFrame;

/* Includes section */
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_GETOPT_LONG
 #include <getopt.h>   // Argument list handling
#else
 #include <unistd.h>
#endif
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
#include "colorspace.h"
#include "writefile.h"
#include "channel.h"
#include "grab.h"
#include "capturey.h"
#include "commands.h"
#include <sys/param.h> // built-in constants and MIN/MAX

/* Globals */
struct GRAB_FORMAT {
    char *name;
    int  bt848;
    int  mul,div;
    int  can_stdout;
    int  can_singlefile;
    int  can_multiplefile;
};
static struct GRAB_FORMAT formats[] = {
    /* file formats */
    { "ppm",  VIDEO_RGB24,     3,1,  0,0,1 },
    { "pgm",  VIDEO_GRAY,      1,1,  0,0,1 },
    { "jpeg", VIDEO_RGB24,     3,1,  0,0,1 },

    /* video */
    { "avi15",VIDEO_RGB15_LE,  2,1,  0,1,0 },
    { "avi24",VIDEO_BGR24,     3,1,  0,1,0 },
    { "mjpeg",VIDEO_MJPEG,     0,1,  0,1,0 },

    /* raw data */
    { "rgb",  VIDEO_RGB24,     3,1,  1,1,1 },
    { "gray", VIDEO_GRAY,      1,1,  1,1,1 },
    { "422",  VIDEO_YUV422,    2,1,  1,1,1 },
    { "422p", VIDEO_YUV422P,   2,1,  1,1,1 },
    { "420p", VIDEO_YUV420P,   3,2,  1,1,1 },

    /* end of table */
    { NULL,   0,               0,0,  0 },
};

/* Mask structure and functions */
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
int winVal(int x, int y, struct window W)
{
  if ( x    >= W.Sx + (float)(y - W.Sy) / (float)W.Dy * (float)W.DDx
       && x <= W.Sx + (float)(y - W.Sy) / (float)W.Dy * (float)W.DDx + W.Dx
       && y >= W.Sy + (float)(x - W.Sx) / (float)W.Dx * (float)W.DDy 
       && y <= W.Sy + (float)(x - W.Sx) / (float)W.Dx * (float)W.DDy + W.Dy   )
    return W.add;
  else return 0;
}
/* Concurrency data passing structure */
struct reading_t
{
  long startV;
  long numV;
  long loadSec;
  long loadUsec;
  long ploadSec;
  long ploadUsec;
};

static int                      calibrate = 0;
static unsigned char           *buffers;
static int                      bufsize;
static int                      BUFCOUNT =  8;
static int                      MAXMASK  = 20;
static int                      MASKSIZE =  7;
static int                      NUMARGS  =  4;
static int                      writer;
static int                      wsync;
static struct timeval           start;
static char*                    tvnorm = NULL;
static char*                    input  = NULL;
static char*                    device = "/dev/video";
static char *filename = NULL;
static char filenamet[256];
static int  single = 1, format = -1, absframes = 1, gray = 0;
static int  fd = -1, width = 320, height = 240, quiet = 0, fps = 30, velo = 0;
static int  deadband = 30, loadTest = 0, loadLevel = -1;
static int  signaled = 0, linelength = 0;
static int  maxedgepix = 320*240;
static int  sub = 1;
int debug = 0, have_dga = 0;
char v4l_conf[] = "";


/* ---------------------------------------------------------------------- */
void
usage(char *prog)
{
    char *h;

    if (NULL != (h = strrchr(prog,'/')))
	prog = h+1;
    
    fprintf(stdout,
	    "%s grabs image(s) from a bt848 card\n"
	    "\n"
	    "usage: %s [ options ]\n"
	    "\n"
	    "options:\n"
	    "  -q          quiet operation\n"
	    "  -c device   specify device              [%s]\n"
	    "  -C calibrate supress bell               [%d]\n"
	    "  -D deadband specify velo filter counts  [%i]\n"
	    "  -f format   specify output format       [%s]\n"
	    "  -S sub      subsample multiple          [%i]\n"
	    "  -s size     specify size                [%dx%d]\n"
	    "  -b buffers  specify # of buffers        [%d]\n"
	    "  -t times    number of frames            [%d]\n"
	    "  -r fps      frame rate                  [%d]\n"
	    "  -o file     output file name            [%s]\n"
	    "  -n tvnorm   set pal/ntsc/secam          [no change]\n"
	    "  -i input    set input source (int)      [no change]\n"
	    "  -j quality  quality for jpeg (not applicable) [%d]\n"
	    "  -g          convert to grayscale (jpeg) [%s]\n"
	    "  -V velocity max pixel change, ignore -o [%d]\n"
	    "  -l load     do a load test, 0=no, 1=full[%d]\n"
	    "\n"
	    "%s will switch the default format depending on the\n"
	    "filename extention (ppm, pgm)\n"
	    "\n"
	    "funny chars:\n"
	    "  +      grabbed picture queued to fifo\n"
	    "  o      grabbed picture not queued (fifo full)\n"
	    "  -      picture written to disk and dequeued from fifo\n"
	    "  s      sync\n"
	    "  xx/yy  (at end of line) xx frames queued, yy frames grabbed\n" 
	    "\n"
	    "formats:\n"
	    "  file formats only:  ppm, pgm\n"
	    "  use streamer for movies or raw\n"
	    "\n"
	    "examples:\n"
	    "  %s -o image.ppm                             write single ppm file\n"
	    "\n"
	    "--\n"
	    "(c) 1998,99 Gerd Knorr <kraxel@goldbach.in-berlin.de>\n",
	    prog, prog, device, calibrate, deadband,
	    (format == -1) ? "none" : formats[format].name,sub,
	    width, height, BUFCOUNT, absframes, fps,
	    filename ? filename : "stdout",
	    jpeg_quality,
	    gray ? "yes" : "no", maxedgepix, loadLevel,
	    prog, prog);
}

/* ---------------------------------------------------------------------- */


static int writerFile(int *talk)
{
    long  args[4];
    int  fd = -1,ret;
    int  fdt = -1;
    int imageNum = 0;

    /* start up new proccess */
    if (0 != (ret = grab_writer_fork(talk)))
	return ret;


    if ( filename )
      {
	fd = open(filename, O_RDWR | O_CREAT, 0666);
	if (-1 == fd )
	  {
	    fprintf(stderr, "writer: open %s: %s\n", filename,
		    strerror(errno));
	    exit(1);
	  }
	fdt = open(filenamet, O_RDWR | O_CREAT, 0666);
	if (-1 == fdt )
	  {
	    fprintf(stderr, "writer: open %s: %s\n", filenamet,
		    strerror(errno));
	    close(fd);
	    exit(1);
	  }
      } 
    else 
      {
	filename = "stdout";
	fd = 1;
      }

    fileBuff  = malloc(absframes*bufsize);
    timeFrame = malloc(2*absframes);
    
    for (;;)
      {
	/* wait for frame */
	switch (read(*talk,args,4*sizeof(long)))
	  {
	  case -1:
	    close(fd);
	    close(fdt);
	    perror("writer: read socket");
	    free(fileBuff);
	    free(timeFrame);
	    exit(1);
	  case 0:
	    /* write out */
	    if (bufsize*absframes != 
		write(fd, fileBuff, bufsize*absframes))
	      {
		fprintf(stderr, "writer: write %s: %s\n",
			filename, strerror(errno));
		free(fileBuff);
		free(timeFrame);
		close(fd);
		close(fdt);
		exit(1);
	      }
	    if ( -1 != fdt )
	      {
		if (2*sizeof(long)*absframes !=
		    write(fdt, timeFrame, absframes*2*sizeof(long)))
		  {
		    fprintf(stderr,"writer: write %s: %s\n",
			    filenamet, strerror(errno));
		    free(fileBuff);
		    free(timeFrame);
		    close(fd);
		    close(fdt);
		    exit(1);
		  }
	      }
	    close(fd);
	    close(fdt);
	    if (!quiet)
	      fprintf(stderr,"writer: done\n");
	    exit(0);
	  } /* switch */


	/* save */
	memcpy(fileBuff+imageNum*bufsize, buffers+args[0]*bufsize, bufsize);
	memcpy(timeFrame+imageNum*2, &args[2], 2*sizeof(long));
	imageNum++;

	/* free buffer */
	if ( sizeof(int) != write(*talk,args,sizeof(long)) )
	  {
	    perror("writer: write socket");
	    free(fileBuff);
	    free(timeFrame);
	    exit(1);
	  }
	if ( !quiet )
	  fprintf(stderr,"-");
      } /* for */

}

static int writerShm(int *talk)
{
    struct window inMask[MAXMASK]; /* Masks */
    struct reading_t reading; /* Info */
    int   numInMask;       /* Number of masks */
    int   maski[bufsize];  /* Index of used pixels */
    int   maskj[bufsize];  /* Index of used pixels */
    int  *mi;              /* Pointer to mask */
    int  *mj;              /* Pointer to mask */
    int   numMask = 0;     /* Number of mask indeces */
    int   maskVal;         /* Local status of mask */
    int   cmd;             /* command from executive */
    long  args[NUMARGS];
    long  pasttimeSec  = 0;/* past image time stamp, sec */
    long  pasttimeUsec = 0;/* past image time stamp, sec */
    int   ret;

    unsigned char *buffersptr;      /* present image */
    unsigned char *bufferspastptr;  /* previous image */
    int   i;               /* local index */
    int   j;               /* local index */
    int   k;               /* local index */
    int   im;              /* local index */
    int   smid;            /* shared memory id  */
    int  *smptr;           /* shared memory location */
    int  *smptrT;          /* temporary pointer */
    long  argspast;        /* last image info */
    int   delta;           /* change since last image */
    int   deltaP;          /* auxilliary calculation variable */
    int   fd_flags;        /* to make stdin non-blocking */
    int   iread;           /* to count bytes returned by reads */
    long  readargs[BUFSIZ];
    int   startV;          /* memory start point */
    int   numV = 0;        /* number of points in shm */
    int   rdgbuflength;    /* length of reading in shm */
    int   index;           /* local index */
    
    /* initialize */
    rdgbuflength = 4*maxedgepix;
    startV     = 0;
    reading.startV = (long)(0);
    reading.numV   = (long)(0);

    /* start up new proccess */
    ret = grab_writer_fork(talk);
    if ( 0 != ret ) return ret;
    
    /* attach shared memory for storing results to executive */
    smid  = shmget(ftok(device, 1), 0, SHM_R | SHM_W | SHM_R>>3 | SHM_R>>6);
    if ( -1 == smid )
      {
	perror("writerShmy shmget");
	exit(1);
      }
    smptr = (int *)shmat(smid, NULL, 0);
    if ( -1 == *smptr )
      {
	perror("writerShmy shmat");
	exit(1);
      }
    argspast = BUFCOUNT-1;

    /* load mask, block on input now */
    iread = read(STDIN_FILENO, &numInMask, sizeof(int));
    if ( -1 == iread
	 && errno != EAGAIN )
      {
	perror("writer: cmd socket");
	exit(1);
      }
    if ( bufsize <= numInMask*MASKSIZE || numInMask >= MAXMASK )
      {
	fprintf(stderr, "writer: mask too large.\n");
	exit(1);
      }
    /* Read the mask */
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
	for (j=0; j<height; j++) for (i=0; i<width; i++)
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
	for (j=0; j<height; j++) for (i=0; i<width; i++)
	  {
	    if ( ( 0 == j%sub ) && ( 0 == i%sub ) )
	      {
		maski[numMask]   = i;
		maskj[numMask++] = j;
	      }
	  }
      }

    /* Setup load test */
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
		*smptrT++ = (height-k)/2 + j;
		*smptrT++ = (width -k)/2 + i;
		*smptrT++ = 100;
		*smptrT++ = 227;
	      }
	  }
      loaddone:
	/* 4 less for corner pix added in Image */
	numV = MAX(MIN(numV, maxedgepix/sub - 5), 2);
      }
    

    /* 
       Setup non-block on stdin so can poll executive.  If things
       work as planned, there will be few wasted frames so this is not
       busy waiting.
    */
    fd_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if ( -1 == fd_flags )
      perror("Could not get flags for stdin");
    else 
      {
	fd_flags |= O_NONBLOCK;
	if ( -1 == fcntl(STDIN_FILENO, F_SETFL, fd_flags) )
	  perror("Could not set flags for stdin");
      }
    
    /* Loop until broken */
    for (;;)
      {
	/* wait for frame (blocking read) */
	/* Read whole contents of buffer to discover latest reading */
	iread = read(*talk, readargs, BUFSIZ);
	switch ( iread )
	  {
	  case -1:
	    perror("writer: read socket");
	    exit(1);
	  case 0:
	    if (!quiet)
	      fprintf(stderr,"writer: done\n");
	    exit(0);
	  }
	iread /= sizeof(long);

	/* Assign */
	args[0] = readargs[iread-4];
	args[1] = readargs[iread-3];
	args[2] = readargs[iread-2];
	args[3] = readargs[iread-1];
	buffersptr     = buffers + args[0]  * bufsize;
	if ( 8 > iread ) bufferspastptr = buffers + argspast * bufsize;
	else    bufferspastptr = buffers + readargs[iread-8] * bufsize;
	argspast = args[0];

	/* write out */
        startV = reading.startV;
	smptrT = &smptr[startV];
	if ( !loadTest ) numV   = 0;
	for (im=0, mi=maski, mj=maskj; im<numMask; im++, mi++, mj++)
	  {
	    index    = (*mj)*width + (*mi);
	    delta    = (int)(buffersptr[index]) - (int)(bufferspastptr[index]);
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

	    /* ignore data if too many pixels changed */
	    if ( numV >= maxedgepix )
	      {
		fprintf(stderr, "*");
		numV = 0;
		goto overflow;
	      }
	  }
      overflow:
	
	/* write buffer information */
	reading.startV    = (long)startV;
	reading.numV      = (long)numV;
	reading.loadSec   = args[2];         /* time sec */
	reading.loadUsec  = args[3];         /* time usec*/
	reading.ploadSec  = pasttimeSec;     /* prev time sec */
	reading.ploadUsec = pasttimeUsec;    /* prev time usec*/

	/* see if exec wants data*/
	iread = read(STDIN_FILENO, &cmd, sizeof(int));
	if ( -1 == iread && errno != EAGAIN )
	  {
	    perror("writer: cmd socket");
	    exit(1);
	  }
	/* else if ( 0 < iread )*/
	/* send to exec even if doesn't want it.  Exec sorts. */
	else
	  {
	    write(STDOUT_FILENO, &reading, sizeof(struct reading_t));
	  }
	pasttimeSec  = args[2];
	pasttimeUsec = args[3];
	
	/* reset synchro buffers */
	reading.numV = (long)(0);
	if ( 0 == iread )
	  fprintf(stderr, ">");

	/* free image buffer */
	if ( sizeof(int) != write(*talk,args,sizeof(long)) )
	  {
	    perror("writer: write socket");
	    exit(1);
	  }
	if ( !quiet )
	  fprintf(stderr,"-");
      }
}

/* ---------------------------------------------------------------------- */

void set_capture(int capture) {}
void ctrlc(int signal)
{
  static char text[] = "^C - one moment please\n";
  if (!velo) write(2,text,strlen(text));
  signaled = 1;
}



int main(int argc, char **argv)
{
    int i;
    // disable digit problems dag 8/10/06
    //    int n;
    int c;
    int count     = 0;
    int queued    = 0;
    int num_args  = 0;

    /* parse options */
    for (;;)
      {
#ifdef HAVE_GETOPT_LONG
	int option_index = 0;
	static struct option long_options[] =
	  {
	    {"help",        0, 0, 'h'},
	    {"version",     0, 0, 'v'},
	    {0, 0, 0, 0}
	  };
	c = getopt_long(argc, argv, "hqdf:S:s:Cc:o:b:t:r:n:i:j:D:V:vgl:",
			long_options, &option_index);
#else
	c = getopt(argc, argv, "hqdf:S:s:Cc:o:b:t:r:n:i:j:D:V:vgl:");
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
	switch (c) {
	case 'q':
	  quiet = 1;
	  break;
	case 'l':
	  loadTest = 1;
	  loadLevel = atoi(optarg);
	  if ( 0 > loadLevel || 1 < loadLevel )
	    {
	      fprintf(stderr, "Bad load level, 0 or 1\n");
	      exit(1);
	    }
	  break;
	case 'd':
	  debug = 1;
	  break;
	case 'f':
	  for (i = 0; formats[i].name != NULL; i++)
	    if (0 == strcasecmp(formats[i].name,optarg))
	      {
		format = i;
		break;
	      }
	  if (formats[i].name == NULL)
	    {
	      fprintf(stderr,"unknown format %s (available:",optarg);
	      for (i = 0; formats[i].name != NULL; i++)
		fprintf(stderr," %s",formats[i].name);
	      fprintf(stderr,")\n");
	      exit(1);
	    }
	  break;
	case 'S':
	  sub = atoi(optarg);
	  break;
	case 's':
	  if (2 != sscanf(optarg,"%dx%d",&width,&height))
	    width = height = 0;
	  break;
	case 'c':
	  device = optarg;
	  break;
	case 'C':
	  calibrate = 1;
	  break;
	case 'o':
	  filename = optarg;
	  // Disable digit problems  DAG 8/10/06
	  /*
	  for (i = 0, n = strlen(filename); i < n; i++)
	    {
	      if (isdigit(filename[i]))
		single = 0;
	    }
	  */
	  if ( single )
	    {
	      strcpy(filenamet, filename);
	      if ( strchr(filenamet, '.') )
		{
		  strcpy(strchr(filenamet, '.'), ".tim");
		}
	    }
	  if (format == -1)
	    {
	      if (strstr(filename,"ppm"))
		format = 0;
	      else
		{
		  fprintf(stderr, "sorry, only ppm support compiled.  Use streamer.\n");
		  exit(1);
		}
	    }
	  break;
	case 'v':
	  fprintf(stdout, "$Id: streamery.c,v 1.1 2007/12/16 21:50:11 davegutz Exp davegutz $ \n\
written by Dave Gutz Copyright (C) 2005");
	  exit(0);
	  break;
	case 'n':
	  tvnorm = optarg;
	  break;
	case 'i':
	  input = optarg;
	  break;
	case 'j':
	  jpeg_quality = mjpeg_quality = atoi(optarg);
	  fprintf(stderr, "sorry, no jpeg support compiled in\n");
	  exit(1);
	  break;
	case 't':
	  absframes = atoi(optarg);
	  break;
	case 'b':
	  BUFCOUNT = atoi(optarg);
	  break;
	case 'r':
	  fps = atoi(optarg);
	  break;
	case 'D':
	  deadband = atoi(optarg);
	  break;
	case 'V':
	  velo = 1;
	  maxedgepix = atoi(optarg);
	  break;
	case 'g':
	  gray = 1;
	  break;
	case 'h':
	  usage(argv[0]);
	  exit(0);
	default:
	  usage(argv[0]);
	  exit(1);
	}
      }
    if (format == -1)
      {
	fprintf(stderr,"no format specified\n");
	exit(1);
      }
    if (gray && (format == 2))
      {
	formats[format].mul = 1;
	formats[format].div = 1;
	formats[format].bt848 = VIDEO_GRAY;
      }
    if (filename == NULL && !formats[format].can_stdout)
      {
	fprintf(stderr,"writing to stdout is not supported for %s\n",
		formats[format].name);
	exit(1);
      }
    if (absframes > 1 && !single && !formats[format].can_multiplefile)
      {
#if 0
	fprintf(stderr,"writing to multiple files is not supported for %s\n",
		formats[format].name);
	exit(1);
#else
	single = 1;
#endif
      }
    if (absframes > 1 && single && !formats[format].can_singlefile)
      {
	fprintf(stderr,"writing to a single file is not supported for %s\n",
		formats[format].name);
	exit(1);
      }


    /* open */
    fd = grabber_open(device,0,0,0,0,0);
    if (grabber->grab_setparams == NULL ||
	grabber->grab_capture   == NULL)
      {
	fprintf(stderr,"%s\ncapture not supported\n",grabber->name);
	exit(1);
      }


    /* modify settings */
    if (input != NULL)
      do_va_cmd(2,"setinput",input);
    if (tvnorm != NULL)
      do_va_cmd(2,"setnorm",tvnorm);

    /* set capture parameters */
    grabber_setparams(formats[format].bt848,&width,&height,
		      &linelength,0);


    /* buffer initialisation */
    if (absframes < BUFCOUNT)
      BUFCOUNT = absframes+2;
    bufsize = width*height*formats[format].mul/formats[format].div;
    if (0 == bufsize)
      bufsize = width*height*3; /* compressed - should be enough */
    if ((buffers = (unsigned char*) grab_initbuffersy(bufsize, BUFCOUNT)) == NULL)
      exit(1);



    /* start up writer */
    if ( velo )
      {
	grab_set_fps(fps);
	writerShm(&writer);
      }
    else
      {
	grab_set_fps(fps);
	writerFile(&writer);
	if ( !calibrate ) fprintf(stderr, "\a");
	sleep(2);
	if ( !calibrate ) fprintf(stderr, "\a");
	fflush(stderr);
      }
    
    /* start up syncer */
    grab_syncer(&wsync, quiet);

    /* catch ^C */
    signal(SIGINT,ctrlc);

    /* main loop */
    gettimeofday(&start,NULL);
    if ( velo && !loadTest )
      {
	for (;!signaled; count++)
	  {
	    grab_putbuffery(quiet,writer,wsync);
	  }
      }
    else
      {
	for (;queued < absframes && !signaled; count++)
	  {
	    queued = grab_putbuffery(quiet,writer,wsync);
	  }
	free(fileBuff);
	free(timeFrame);
      }
    
    /* done */
    grabber->grab_close();
    if (!quiet)
      fprintf(stderr,"\n");
    if (single && formats[format].can_singlefile)
      {
	fprintf(stderr,"\a");
	fflush(stderr);
      }
    close(fd);
    shutdown(writer,1);
    close(wsync);
    close(writer);
    return 0;
}
