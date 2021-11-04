#ifndef UMP_COMMON_H
#define UMP_COMMON_H 1

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>

#if STDC_HEADERS
#  include <stdlib.h>
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif /*STDC_HEADERS*/

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif /*HAVE_ERRNO_H*/
#ifndef errno
/* Some systems #define this! */
extern int errno;
#endif


#ifdef NO_BOUNDS_CHECK
 #define TNT_NO_BOUNDS_CHECK
#else
 #define BOUNDS_CHECK
 #define TNT_BOUNDS_CHECK
#endif

#define   SIGN(A)         ((A) < (0) ? (-1): (1))

#endif /* !UMP_COMMON_H */

/* To debug real time operation */
#define VERBOSE



