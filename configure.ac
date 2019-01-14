#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([wut-tools],[1.0.0],[https://github.com/switchbrew/switch-tools/issues])
AC_CONFIG_SRCDIR([src/elf2rpl/main.cpp])
AC_CONFIG_MACRO_DIRS([m4])

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

AX_CXX_COMPILE_STDCXX_14

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

CFLAGS="$CFLAGS -std=gnu99"

AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_LIBS)
AC_SUBST(NET_LIBS)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
