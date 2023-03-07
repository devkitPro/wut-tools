#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([wut-tools],[1.3.4],[https://github.com/devkitPro/wut-tools/issues])
AC_CONFIG_SRCDIR([src/elf2rpl/main.cpp])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_CANONICAL_HOST
AC_CANONICAL_BUILD
AC_PROG_LIBTOOL

AC_SYS_LARGEFILE

AX_CXX_COMPILE_STDCXX_14(noext, mandatory)

PKG_CHECK_MODULES([ZLIB], zlib, [
  AC_DEFINE([HAVE_LIBZ], [1], [Define if using zlib.])
])

NET_LIBS=""

case "$host" in
   *-*-mingw*)
      NET_LIBS="-lws2_32"
      CFLAGS="$CFLAGS -D__USE_MINGW_ANSI_STDIO"
      ;;
esac

case "$host" in
   *-apple-darwin*)
      STDCPP_LIBS="-lc++"
      ;;

   *)
      STDCPP_LIBS="-lstdc++"
      ;;

esac

AC_CHECK_HEADER([FreeImage.h])

AC_MSG_CHECKING([for libfreeimage])
save_LIBS="$LIBS"
LIBS="-lfreeimage ${LIBS} ${NET_LIBS} ${STDCPP_LIBS} -lm"
AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[#include <FreeImage.h>]],
        [[FreeImage_DeInitialise()]])],
        [freeimage_result=yes],
        [freeimage_result=no])
AC_MSG_RESULT([$freeimage_result])
LIBS="$save_LIBS"
if test "x$freeimage_result" = "xyes"; then
        FREEIMAGE_LIBS="-lfreeimage ${NET_LIBS} ${STDCPP_LIBS} -lm"
else
         AC_MSG_ERROR(['libfreeimage' not found])
fi

CFLAGS="$CFLAGS -std=gnu99"

AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_LIBS)
AC_SUBST(FREEIMAGE_LIBS)
AC_SUBST(NET_LIBS)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
