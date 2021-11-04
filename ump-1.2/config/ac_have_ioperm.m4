dnl @synopsis AC_HAVE_IOPERM
dnl
dnl If the compiler can access sys/io.h then set
dnl HAVE_IOPERM.
dnl
dnl @version $Id: ac_have_ioperm.m4,v 1.1 2010/01/10 11:21:57 davegutz Exp $
dnl @author Dave Gutz
dnl
AC_DEFUN([AC_HAVE_IOPERM],
[AC_CACHE_CHECK(whether the compiler supports ioperm(),
ac_have_ioperm,
[AC_TRY_COMPILE([#include <sys/io.h>],
[ioperm(0x0378,1,1);],
 ac_have_ioperm=yes, ac_have_ioperm=no)
])
if test "$ac_have_ioperm" = yes; then
   AC_DEFINE(HAVE_IOPERM,,[define if compiler supports ioperm() with sys/io.h])
fi
])


