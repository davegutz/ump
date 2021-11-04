// Memory exception handling used with new.h
// Usage: add line   'set_new_handler(memory_err);' in main()
/* 	$Id: memory_err.h,v 1.1 2006/08/16 22:07:19 davegutz Exp $	 */
#ifndef memory_err_h
#define memory_err_h 1

#ifdef HAVE_NEW_HANDLER
#ifdef BACKWARD
#  include <fstream.h>   // for cerr
#else
#  include <iostream>     // for cerr
#endif
// Memory exception handling used with new.h
extern void (*set_new_handler(void (*memory_err)()))();
void memory_err()
{
  cerr << "Memory allocation error.  Try smaller image.  Quitting." << endl;
  exit(1);
}
#endif  // HAVE_NEW_HANDLER

#endif // memory_err_h
