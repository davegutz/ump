dnl @synopsis AC_CXX_HAVE_FMATH
dnl
dnl If the compiler can access sinf, etc, set
dnl HAVE_FMATH.
dnl
dnl @version $Id: ac_cxx_have_fmath.m4,v 1.2 2001/12/08 17:23:23 dgutz Exp $
dnl @author Dave Gutz
dnl
AC_DEFUN([AC_CXX_HAVE_FMATH],
[AC_CACHE_CHECK(whether the compiler supports C++ sinf etc,
ac_cxx_have_fmath,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 save_LIBS="$LIBS"
 LIBS="-lm $LIBS"
 AC_TRY_COMPILE([#include <math.h>],
[sinf(0);],
 ac_cxx_have_fmath=yes, ac_cxx_have_fmath=no)
 AC_LANG_RESTORE
 LIBS="$save_LIBS"
])
if test "$ac_cxx_have_fmath" = yes; then
   AC_DEFINE(HAVE_FMATH,,[define if compiler supports C++ sinf, etc])
fi
])


