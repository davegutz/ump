// 	$Id: mm2ppm.cc,v 1.2 2010/01/10 12:07:03 davegutz Exp $	
// Convert a mmio file (.mm) into a ppm image

// Includes section
using namespace std;
#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif
#include <common.h>
#include <math.h>   // for fabs on some systems
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#ifdef HAVE_GETOPT_LONG
 #include <getopt.h>
#else
 #include <unistd.h>
#endif
#include <string.h>
#ifdef HAVE_MALLOC_H
 #include <malloc.h>
#endif
extern "C"
{
 #include <mmio.h>
}
#include <Mask.h>    // Mask
//#include <PixelLoc.h>  // Pixel stuff
//#include <Segment.h>   // Point, Line stuff
//#include <Image.h>

// Globals
int verbose   = 0;
int calibrate = 0;
int dump      = 0;
timeval globalProcStart; 

void assignRGBmm(const int request, int *RED, int *GREEN, int *BLUE)
{
  switch (request)
    {
    case 0: *RED = 255; *GREEN = 165; *BLUE =   0; break; // Orange
    case 1: *RED = 255; *GREEN =   0; *BLUE =   0; break; // Red
    case 2: *RED =   0; *GREEN =   0; *BLUE = 255; break; // Blue
    case 3: *RED =   0; *GREEN = 255; *BLUE =   0; break; // Lime
    case 4: *RED =  80; *GREEN =   0; *BLUE =   0; break; // Maroon 
    case 5: *RED = 255; *GREEN =   0; *BLUE = 255; break; // Fuchsia, Magenta 
    case 6: *RED =   0; *GREEN = 255; *BLUE = 255; break; // Aqua, Cyan
    case 7: *RED =   0; *GREEN =  80; *BLUE =  80; break; // Teal 
    case 8: *RED =  80; *GREEN =  80; *BLUE =   0; break; // Olive
    case 9: *RED =   0; *GREEN =  80; *BLUE =  80; break; // Navy
    case 10:*RED =   0; *GREEN =  80; *BLUE =   0; break; // Green
    case 11:*RED =  80; *GREEN =  80; *BLUE =  80; break; // Gray
    case 12:*RED = 192; *GREEN = 192; *BLUE = 192; break; // Silver
    case 13:*RED = 154; *GREEN = 205; *BLUE =  32; break; // Yellow Green
    case 14:*RED = 139; *GREEN =   0; *BLUE =   0; break; // Dark Red
    default:*RED =   0; *GREEN =   0; *BLUE =   0;        // Black
    }
}

void usage(char *prog)
{
  char *h;
  if (NULL != (h = strrchr(prog,'/')))
    prog = h+1;
  
  fprintf(stdout,"\
%s reads in MatrixMarket format and draws ppm image.\n\
usage:  \n\
%s [ options ] <filename.mm>\n\n\
options:\n\
  -h|--help           display this banner                     \n\
  -m|--maskfile <maskfile>          Apply mask in file        \n\
  -V|--version        print version and exit    \n\
  -v|--verbose  <level>    \n", prog, prog);
}

// Memory exception handling used with new.h
void memory_err()
{
  cerr << "mm2ppm: " << __FILE__ << " : " << __LINE__<< " : "  
       << "memory allocation error.   Quitting" << endl;
  exit(1);
}

// ----------------------------------------------------------------------
int main(int argc, char **argv)
{
  set_new_handler(memory_err);  // Set up exception handler.
  char c;
  char filename[NAME_MAX];
  int ret_code;
  MM_typecode matcode;
  FILE *f;
  int n, m, nz;   
  int *I, *J;
  double *val;
  int masking = 0;     // Flag to indicate performing mask
  char *maskFile="";
  Mask *mask;
  Pixel **picture;
  char *vmsg="$Id: mm2ppm.cc,v 1.2 2010/01/10 12:07:03 davegutz Exp $";

  // parse options
  int num_args=0;
  while (1)
    {
#ifdef HAVE_GETOPT_LONG
      int option_index = 0;
      static struct option long_options[] =
      {
	{"help",        0, 0, 'h'},
	{"maskfile",    1, 0, 'm'},
	{"version",     0, 0, 'V'},
	{"verbose",     1, 0, 'v'},
	{0, 0, 0, 0}
      };
      c = getopt_long(argc, argv, "hm:Vv:", long_options, &option_index);
#else
      c = getopt(argc, argv, "hm:Vv:");
#endif // HAVE_GETOPT_LONG
      if ( '?' == c ) exit(1);
      if (-1 == c) break;
      num_args++;
      switch (c)
	{
	case 'h':
	  usage(argv[0]);
	  exit(0);
	  break;
	case 'm':
	  delete maskFile;
	  maskFile = new char[MAX_CANON];
          strcpy(maskFile, optarg);
	  masking=1;
          break;
	case 'V':
	  cout << vmsg << endl;
	  exit(0);
	case 'v':
	  verbose = atoi(optarg);
	  if ( 0 > verbose || 10 < verbose )
	    {
	      cerr << "ERROR(mm2ppm):  " << __FILE__ << " : " << __LINE__
		   << " : "  << "bad verbose specified = " << verbose << endl;
	      exit(1);
	    }
	  break;
	default:
	  usage(argv[0]);
	  if ( 2> num_args ) exit(1);
	  break;
	}
    }

  // Setup files
  if ( argc-optind != 1 )
    {
      cerr << "ERROR(mm2ppm):  " << __FILE__ << " : " << __LINE__
	   << " : "  << "some option is missing or bad" << endl;
      exit(1);
    }
  strcpy(filename, argv[optind]);
  char *dotp;
  char *ext;
  if ( NULL == (dotp = strrchr(filename, '.')) )
    {
      cerr << "ERROR(mm2ppm): " << __FILE__ << " : " << __LINE__<< " : "  
	   << "must have extension, e.g. .mm.  Got " << filename << endl;
      exit(1);
    }
  ext = dotp+1;
  if ( strcmp(ext, "mm") )
    {
      cerr << "ERROR(mm2ppm): " << __FILE__ << " : " << __LINE__<< " : "  
	   << "must have extension mm\n";
      exit(1);
    }
  if ( NULL == (f = fopen(filename, "r")) )
    {
      cerr << "ERROR(mm2ppm): " << __FILE__ << " : " << __LINE__<< " : "  
	   << "could not open file " << filename << endl;
      exit(1);
    }
  if ( 0 != mm_read_banner(f, &matcode) )
    {
      cerr << "ERROR(mm2ppm): " << __FILE__ << " : " << __LINE__<< " : "  
	   << "could not process Matrix Market banner\n";
      exit(1);
    }

  //  This is how one can screen matrix types if their application
  //  only supports a subset of the Matrix Market data types.
  if ( mm_is_complex(matcode) &&
       mm_is_matrix (matcode) &&
       mm_is_sparse (matcode)   )
    {
      cerr << "ERROR(mm2ppm): " << __FILE__ << " : " << __LINE__<< " : "  
	   << "sorry, this application does not support\n"
	   << "Matrix Market type: " << mm_typecode_to_str(matcode) << endl;
      exit(1);
    }

  // find out size of sparse matrix ....
  if ( (ret_code = mm_read_mtx_crd_size(f, &n, &m, &nz)) !=0 ) exit(1);

  // reseve memory for matrices 
  I   = new int   [nz];
  J   = new int   [nz];
  val = new double[nz];

  // Initialize gray.
  unsigned char *img = new unsigned char[n*m*3];
  memset(img, 80, n*m*3*sizeof(unsigned char)); 

  // Mask set black
  if ( masking )
    {
      int bufsize = n*m;
      picture = new Pixel*[bufsize];
      for (int j=0; j<bufsize; j++)
	{
	  picture[j]     = new Pixel();
	}
      mask    = new Mask(maskFile, n, m, bufsize);
      mask->loadMasksFrom(maskFile);
      mask->applyMasksTo(picture, n, m, 1);
      for ( int j=0; j<m; j++ ) for ( int i=0; i<n; i++ )
	{
	  int index = j*n+i;
	  if ( !picture[index]->inMask )
	    {
	      img[index*3    ] = 0;
	      img[index*3 + 1] = 0;
	      img[index*3 + 2] = 0;
	    }
	}
    }

  // NOTE: when reading in doubles, ANSI C requires the use of the "l"  
  // specifier as in "%lg", "%lf", "%le", otherwise errors will occur 
  // (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            
  for (int i=0; i<nz; i++)
    {
      fscanf(f, "%d %d %lg\n", &I[i], &J[i], &val[i]);
      I[i]--; J[i]--;
    }
  if ( f != stdin ) fclose(f);

  // Make ppm file
  strcpy(ext, "ppm");
  if ( NULL == (f = fopen(filename, "w")) )
    {
      cerr << "ERROR(mm2ppm): " << __FILE__ << " : " << __LINE__<< " : "  
	   << "could not open file " << filename << endl;
      exit(1);
    }
  fprintf(f, "P6\n");
  fprintf(f, "%d %d\n", n, m);
  fprintf(f, "255\n");

  // Temporary image
  for (int j = 0; j < nz ; j++)
    {
      int index   = J[j]*n + I[j];
      int RED, GREEN, BLUE;
      assignRGBmm((int)val[j], &RED, &GREEN, &BLUE);
      img[index*3      ] = (unsigned char)(RED);
      img[index*3   + 1] = (unsigned char)(GREEN);
      img[index*3   + 2] = (unsigned char)(BLUE);
    }
  unsigned char *imgp = img;
  for (int j = 0 ; j < m ; j++)
    {
      fwrite(imgp, n, 3, f);
      imgp += 3*n;
    }
  
  fclose(f);

  return 0;
}




