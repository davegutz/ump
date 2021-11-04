dnl @synopsis AC_HAVE_ENDIAN
dnl
dnl If the compiler can access endian.h then set
dnl HAVE_ENDIAN.
dnl
dnl @version $Id: ac_have_endian.m4,v 1.1 2010/01/10 11:17:49 davegutz Exp $
dnl @author Dave Gutz
dnl
AC_DEFUN([AC_HAVE_ENDIAN],
[AC_CACHE_CHECK(whether the compiler supports endian(),
ac_have_endian,
[AC_TRY_COMPILE([#include <endian.h>],
 ac_have_endian=yes, ac_have_endian=no)
])
if test "$ac_have_endian" = yes; then
   AC_DEFINE(HAVE_ENDIAN,,[define if compiler supports endian.h])
fi
])
