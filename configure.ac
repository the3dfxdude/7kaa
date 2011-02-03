#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([7kaa], [2.14.3], [skfans-devel@lists.sourceforge.net])
AC_CONFIG_SRCDIR([src/client/AM.cpp])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign])
ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
dnl AC_CHECK_HEADERS([])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
dnl AC_CHECK_FUNCS([])

use_openal=yes

AM_CONDITIONAL([AUDIO_OPENAL], [test "$use_openal" = yes])
AM_CONDITIONAL([AUDIO_DSOUND], [false])
AM_CONDITIONAL([INPUT_DINPUT], [false])
AM_CONDITIONAL([INPUT_NONE], [false])
AM_CONDITIONAL([INPUT_SDL], [true])
AM_CONDITIONAL([NET_NONE], [false])
AM_CONDITIONAL([NET_SDLNET], [true])
AM_CONDITIONAL([VIDEO_DDRAW], [false])
AM_CONDITIONAL([VIDEO_NONE], [false])
AM_CONDITIONAL([VIDEO_SDL], [true])

AS_IF([test "$use_openal" = yes], [
  AM_PATH_OPENAL([], [
    AC_ERROR([OpenAL not found])
  ])

  AC_SUBST([OPENAL_CFLAGS])
  AC_SUBST([OPENAL_LIBS])

  LIBS="$LIBS $OPENAL_LIBS"
])

use_windows_headers=no

AS_IF([test "$use_windows_headers" = no], [
  AC_DEFINE([NO_WINDOWS], [1], [Define to 1 if not using Windows headers])
])

AC_DEFINE([NO_ASM], [1], [
  Define to 1 if not using assembly routines or if NO_WINDOWS is defined
])

AC_DEFINE([USE_SDL], [1], [Define to 1 if using SDL as the video backend])

AS_IF([test "$use_openal" = yes], [
  AC_DEFINE([USE_OPENAL], [1], [
    Define to 1 if using OpenAL as the audio backend
  ])
])

AC_DEFINE([USE_SDLNET], [1], [
  Define to 1 if using SDL_net as the netplay backend
])

AC_DEFINE([AMPLUS], [1], [
  Define to 1 if building Ancient Adversaries
])

COMMON_CXXFLAGS="-include \"\$(top_srcdir)/include/config.h\""

CXXFLAGS="$CXXFLAGS $COMMON_CXXFLAGS"

AC_CONFIG_FILES([
  Makefile
  include/Makefile
  src/Makefile
  src/audio/Makefile
  src/audio/dsound/Makefile
  src/audio/openal/Makefile
  src/client/Makefile
  src/common/Makefile
  src/file/Makefile
  src/imgfun/Makefile
  src/imgfun/asm/Makefile
  src/input/Makefile
  src/input/dinput/Makefile
  src/input/none/Makefile
  src/input/sdl/Makefile
  src/netplay/Makefile
  src/netplay/common/Makefile
  src/netplay/none/Makefile
  src/netplay/sdl_net/Makefile
  src/server/Makefile
  src/video/Makefile
  src/video/common/Makefile
  src/video/ddraw/Makefile
  src/video/none/Makefile
  src/video/sdl/Makefile
])

AC_OUTPUT
