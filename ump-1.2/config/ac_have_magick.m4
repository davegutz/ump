dnl @synopsis AC_HAVE_MAGICK
dnl
dnl If the compiler can access Image Magick libs set:
dnl HAVE_MAGICK.
dnl
dnl @version $Id: ac_have_magick.m4,v 1.4 2002/06/29 20:32:50 dgutz Exp $
dnl @author Dave Gutz
dnl

AC_DEFUN([AC_HAVE_MAGICK],
[save_LIBS="$LIBS"
MAGICK_LIBS="-L/usr/X11R6/lib -lMagick"
LIBS="$MAGICK_LIBS $LIBS"
acx_magick_ok=no
AC_MSG_CHECKING([for the ImageMagick library])
AC_TRY_LINK_FUNC(GetImageInfo, [acx_magick_ok=yes])
AC_TRY_LINK_FUNC(ReadImage, ,[acx_magick_ok=no])
AC_TRY_LINK_FUNC(GetAnnotateInfo, ,[acx_magick_ok=no])
AC_TRY_LINK_FUNC(AnnotateImage, ,[acx_magick_ok=no])
AC_TRY_LINK_FUNC(BorderImage, ,[acx_magick_ok=no])
AC_TRY_LINK_FUNC(DestroyImage, ,[acx_magick_ok=no])
AC_TRY_LINK_FUNC(WriteImage, ,[acx_magick_ok=no])
AC_MSG_RESULT([$acx_magick_ok])
LIBS="$save_LIBS"
if test x"$acx_magick_ok" = xyes; then
  AC_DEFINE(HAVE_MAGICK, 1,
            [Define if you have Magick libraries and header files.])
  :
else
  MAGICK_LIBS=" "
  AC_MSG_RESULT([Magick not found.  Just can't use showppms & showmms.])
fi
AC_SUBST(MAGICK_LIBS)

])


