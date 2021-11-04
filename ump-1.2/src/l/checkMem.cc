// Memory monitor functions
#include <checkMem.h>

#ifdef HAVE_SBRK


// Get present memory usage
int getMem()
{
  return (int)sbrk(0);
}
// Print present memory usage
void printMem(const char *comment)
{
  cerr << getMem() << "  " << comment << endl;
}
//Check for change in present memory usage
void checkMem(const char *comment)
{
  int newMem = getMem();
  if ( 0 == mem ) mem = newMem;
  if ( newMem != mem )
    {
#ifdef VERBOSE
      if ( 0 < verbose ) cout <<comment << ": memory usage changed by "
			  << newMem - mem << endl;
#endif
      mem = newMem;
    }
}

#else
#error "ERROR(checkMem.cc):  sbrk() not available.  Undefine DEBUG_MEM in common.h"
#endif // HAVE_SBRK
