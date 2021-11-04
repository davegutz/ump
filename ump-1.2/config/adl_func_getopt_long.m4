dnl @synopsis adl_FUNC_GETOPT_LONG
dnl
dnl Check for getopt_long support.
dnl
dnl This assume that the standard getopt.h file (from GNU libc) is
dnl available as lib/gnugetopt.h.  If needed, this file will be linked
dnl as getopt.h, but we want to default to the system's getopt.h file.
dnl (See http://sources.redhat.com/ml/automake/2000-09/msg00041.html
dnl for an explanation about why using the system's getopt.h file
dnl is important.)
dnl
dnl @version $Id: adl_func_getopt_long.m4,v 1.1 2002/01/04 23:04:19 dgutz Exp $
dnl @author Alexandre Duret-Lutz <duret_g@epita.fr>
dnl Modified 2002/01/04 Dave Gutz <davegutz@earthlink.net> for old autoconf

AC_DEFUN([adl_FUNC_GETOPT_LONG],
 [# clean out junk possibly left behind by a previous configuration
  rm -f lib/getopt.h
  # Check for getopt_long support
  AC_CHECK_HEADER([getopt.h])
  AC_CHECK_FUNCS([getopt_long],,
   [# FreeBSD has a gnugetopt library for this
    AC_CHECK_LIB([gnugetopt],[getopt_long],[AC_DEFINE([HAVE_GETOPT_LONG])])])])
