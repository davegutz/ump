dnl @synopsis AC_CXX_HAVE_NEW_HANDLER
dnl
dnl If the compiler can access set_new_handler set
dnl HAVE_NEW_HANDLER.
dnl
dnl @version $Id: ac_cxx_have_new_handler.m4,v 1.3 2002/01/03 00:55:30 dgutz Exp $
dnl @author Dave Gutz
dnl
AC_DEFUN([AC_CXX_HAVE_NEW_HANDLER],
[AC_CACHE_CHECK(whether the compiler supports C++ set_new_handler,
ac_cxx_have_new_handler,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <fstream.h>
extern void (*set_new_handler(void (*memory_err)()))();
void memory_err(){}],
[set_new_handler(memory_err);],
 ac_cxx_have_new_handler=yes, ac_cxx_have_new_handler=no)
 AC_LANG_RESTORE
])
if test "$ac_cxx_have_new_handler" = yes; then
   AC_DEFINE(HAVE_NEW_HANDLER,,[define if compiler supports C++ set_new_handler])
fi
])


