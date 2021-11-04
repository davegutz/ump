/*
 * Simple parallel port output control program for Linux
 * Written and copyright by Tomi Engdahl 1998
 * (e-mail: tomi.engdahl@hut.fi)
 *
 * The program output the data value to PC parallel port data pins
 * (default lpt1 I/O address 0x378). The data values are given as the
 * command line parameter to the program. The number can be
 * in decimal (0..255) or hexadecimal format (0x00..0xFF).
 *
 */

#include <common.h>
#ifdef HAVE_GETOPT_LONG
 #include <getopt.h>   // Argument list handling
#else
 #include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_IOPERM
 #include <sys/io.h>
#endif
#include <string.h>

#define base 0x0378           /* printer port base address */


// Program usage
void usage(char *prog)
{
    char *h;

    if ( NULL != (h = strrchr(prog,'/')) )
	prog = h+1;
    fprintf(stdout,
	    "%s:  writes values to parallel port.  \n"
	    "usage:\n %s Value \n"
	    "\n"
	    "options:\n"
	    "  -h, --help         this screen                     \n"
	    "  -V, --version      version                         \n"
	    "                                                     \n"
	    "  Pin     2   3   4   5   6   7   8   9              \n"
	    "  Bit    D0  D1  D2  D3  D4  D5  D6  D7              \n"
	    "  Value   1   2   4   8  16  32  64 128              \n\n"
	    "  Pins 18 - 25 are grounds                           \n"
	    "  Pins 1,14,16, & 17 are 5 vdc sources               \n"
	    "  Use low current LEDs (2 mA) and drop a resistor    \n"
	    "  so no more than 2.6 mA is drawn at 2.4 vdc total drop. \n\n"
	    "examples:                                            \n"
	    "%s 3    # lights up Pins 2 & 3                       \n"
	    "--                                                   \n"
	    "(c) 2005 Dave Gutz <davegutz@alum.mit.edu>      \n",
	    prog, prog, prog);
}


int main(int argc, char **argv)
{                    
  int value;
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
	{"version",     0, 0, 'V'},
	{0, 0, 0, 0}
      };
      c = getopt_long(argc, argv, "hV", long_options, &option_index);
#else
      c = getopt(argc, argv, "hV");
#endif // HAVE_GETOPT_LONG
      if ( -1 == c ) break;
      num_args++;
      switch (c) 
	{
	case 'h':
	  usage(argv[0]);
	  exit(0);
	  break;
	case 'V':
	  fprintf(stdout, "lptout: $Id: lptout.c,v 1.2 2010/01/10 11:29:41 davegutz Exp $ \n\
written by Dave Gutz Copyright (C) 2005");
	  exit(0);
	  break;
	default:
	  usage(argv[0]);
	  if ( 2 > num_args ) exit(1);
	  break;
	}
    }


  if (argc!=2)
    {
      fprintf(stderr, "ERROR(lptout): Wrong number of arguments. \n\
Needs one argument which is number between 0 and 255.\n");
      exit(1);
    }


  if (sscanf(argv[1],"%i",&value)!=1)
    {
      fprintf(stderr, "ERROR(lptout): Parameter is not a number.\n");
      exit(1);
    }
  if ((value<0) || (value>255))
    {
      fprintf(stderr,
	      "ERROR(lptout): Invalid value. \n\
Number must be between 0 and 255\n");
      exit(1);
    }
#ifdef HAVE_IOPERM
  if (ioperm(base,1,1))
    {
      fprintf(stderr, "ERROR(lptout): Couldn't get the port at %x\n", base);
      exit(1);
    }
  outb((unsigned char)value, base);
#endif
  return(0);
}                                            
