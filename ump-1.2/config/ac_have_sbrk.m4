dnl @synopsis AC_HAVE_SBRK
dnl
dnl If the compiler can access sbrk() then set
dnl HAVE_SBRK.
dnl
dnl @version $Id: ac_cxx_have_new_handler.m4,v 1.2 2001/12/08 17:23:50 dgutz Exp $
dnl @author Dave Gutz
dnl
AC_DEFUN([AC_HAVE_SBRK],
[AC_CACHE_CHECK(whether the compiler supports sbrk(),
ac_have_sbrk,
[AC_TRY_COMPILE([#include <unistd.h>],
[sbrk(0);],
 ac_have_sbrk=yes, ac_have_sbrk=no)
])
if test "$ac_have_sbrk" = yes; then
   AC_DEFINE(HAVE_SBRK,,[define if compiler supports sbrk()])
fi
])


