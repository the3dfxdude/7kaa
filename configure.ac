#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([7kaa], [2.15.0], [the3dfxdude@gmail.com], [],
	[http://7kfans.com/])
AC_CONFIG_SRCDIR([src/AM.cpp])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign])
ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Utility definitions

dnl SEARCH_LIB_FLAGS(FUNCTION, LIB-FLAGS,
dnl		     [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
AC_DEFUN([SEARCH_LIB_FLAGS], [
  ac_save_LIBS="$LIBS"
  ac_check_libs_found=no
  AS_FOR([], [ac_lib], [$2], [
    LIBS="$ac_lib $ac_save_LIBS"
    AC_MSG_CHECKING([for $1 in $ac_lib])
    AC_LINK_IFELSE([AC_LANG_CALL([], [$1])], [
      AC_MSG_RESULT([yes])
      ac_check_libs_found=yes
      $3
      break
    ], [
      AC_MSG_RESULT([no])
    ])
  ])
  AS_IF([test "$ac_check_libs_found" = no], [
    $4
  ])
])

dnl EVAL_SHELL_VAR(NAME)
dnl Recursively does "eval NAME=$NAME" until $NAME stays the same
AC_DEFUN([EVAL_SHELL_VAR], [
  AS_VAR_PUSHDEF([prev], [ac_previous_$1])
  AS_VAR_SET([prev], [$$1])
  while true; do
    eval $1="\"$$1\""
    AS_VAR_IF([prev], ["$$1"], [
      break
    ])
    AS_VAR_SET([prev], [$$1])
  done
  AS_VAR_POPDEF([prev])
])

# Checks for programs.
CXXFLAGS="$CXXFLAGS -fsigned-char"
AC_LANG(C++)
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_LN_S
PKG_PROG_PKG_CONFIG

AX_CXX_COMPILE_STDCXX_11(,[mandatory])

AC_MSG_CHECKING(whether char is signed)
AC_COMPILE_IFELSE([AC_LANG_SOURCE(
#include <limits.h>
#if CHAR_MIN == 0
#error
#endif
  )],[
    AC_MSG_RESULT(yes)
  ],[
    AC_MSG_RESULT(no)
    AC_ERROR([The compiler must interpret char as signed])
  ]
)

# Checks for header files.
dnl AC_CHECK_HEADERS([])

# Checks for typedefs, structures, and compiler characteristics.
AC_CANONICAL_HOST

AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([_NSGetExecutablePath])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19])

# Configuration variables

audio_backend=openal # openal
input_backend=sdl    # sdl
video_backend=sdl    # sdl
use_windows=no
use_mingw=no
jwasm_fmt="-elf -zt0"
enable_enet=yes

# Set in argument handlers:
# enable_asm=no
# enable_curl=yes
# enable_debug=no
# with_wine=no
# enable_dynamic_support_libs=no

# Configuration options

AC_ARG_ENABLE([asm],
  AS_HELP_STRING([--enable-asm], [enable x86 asm optimizations]),
  [], [enable_asm=no]
)

AC_ARG_ENABLE([curl],
  AS_HELP_STRING([--disable-curl], [disable curl support]),
  [], [enable_curl=yes]
)

AC_ARG_ENABLE([debug],
  AS_HELP_STRING([--enable-debug], [enable debugging features]),
  [], [enable_debug=no]
)

AC_ARG_ENABLE([network],
  AS_HELP_STRING([--disable-network], [disable network support]),
  [], [enable_network=yes]
)

#AC_ARG_WITH([wine],
#  AS_HELP_STRING([--with-wine], [build with winelib using wineg++]),
#  [], [with_wine=no]
#)

AC_ARG_ENABLE([dynamic-support-libs],
  AS_HELP_STRING([--enable-dynamic-suppport-libs],
		 [do not link statically with libgcc and libstdc++ on mingw]),
  [], [enable_dynamic_support_libs=no]
)


# Select which backends to build

AS_CASE(["$host"],
  [*-mingw*], [
    use_windows=yes
    use_mingw=yes
    jwasm_fmt=-coff
])

AM_CONDITIONAL([AUDIO_OPENAL], [test "$audio_backend" = openal])
AM_CONDITIONAL([INPUT_SDL],    [test "$input_backend" = sdl])
AM_CONDITIONAL([VIDEO_SDL],    [test "$video_backend" = sdl])
AM_CONDITIONAL([USE_WINDOWS],  [test "$use_windows"   = yes])
AM_CONDITIONAL([USE_ASM],      [test "$enable_asm"    = yes])
AM_CONDITIONAL([USE_CURL],     [test "$enable_curl"   = yes])


# Wine set-up

AS_IF([test "$with_wine" = yes], [
  winecxx=$(which wineg++)
  AS_IF([test -z "$winecxx"], [
    AC_ERROR([wineg++ not found])
  ])
  AC_MSG_CHECKING([the wine prefix])
  wine_prefix=$(echo "$winecxx" | sed 's/\/bin\/wineg++$//')
  AC_MSG_RESULT([$wine_prefix])
  CXX="wineg++"
  CXXFLAGS="$CXXFLAGS -I\"$wine_prefix/include/wine/msvcrt\""
  CXXFLAGS="$CXXFLAGS -I\"$wine_prefix/include/wine/windows\""
])


# Jwasm assembler set-up
AS_IF([test "$enable_asm" = yes], [
  JWASM_CMD=$(which jwasm)
  AS_IF([test -z "$JWASM_CMD"], [
    AC_ERROR([jwasm not found])
  ])
  AC_ARG_VAR([JWASM_CMD],[jwasm command])
  AC_SUBST([JWASM_FMT],["$jwasm_fmt"])
])


# Library dependencies

AS_IF([test "$audio_backend" = openal], [
  PKG_CHECK_MODULES([OPENAL], [openal], [
    AC_DEFINE_UNQUOTED([OPENAL_AL_H], [<AL/al.h>],
      [Define to the equivalent of <AL/al.h> on your system])
    AC_DEFINE_UNQUOTED([OPENAL_ALC_H], [<AL/alc.h>],
      [Define to the equivalent of <AL/alc.h> on your system])
  ], [
    AM_PATH_OPENAL([], [
      AC_ERROR([OpenAL not found])
    ])
  ])

  CXXFLAGS="$CXXFLAGS $OPENAL_CFLAGS"
  LIBS="$OPENAL_LIBS $LIBS"
])

AS_IF([test "$input_backend" = sdl \
       || test "$video_backend" = sdl], [
  AM_PATH_SDL2([], [], [
    AC_ERROR([SDL not found])
  ])

  CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
  LIBS="$SDL_LIBS $LIBS"
])

AS_IF([test "$enable_enet" = yes], [
  PKG_CHECK_MODULES([ENET], [libenet], [], [
    SEARCH_LIB_FLAGS([enet_initialize], ["-lenet -lws2_32 -lwinmm" -lenet],, [
      AC_ERROR([enet not found])
    ])
  ])

  CXXFLAGS="$CXXFLAGS $ENET_CFLAGS"
  LIBS="$LIBS $ENET_LIBS"
])

AS_IF([test "$enable_curl" = yes], [
  PKG_CHECK_MODULES([CURL], [libcurl], [], [
    AC_ERROR([curl not found])
  ])

  CXXFLAGS="$CXXFLAGS $CURL_CFLAGS"
  LIBS="$LIBS $CURL_LIBS"
])

AS_IF([test "$use_windows" = yes], [
  LIBS="-lole32 -lmsvcrt -lwinmm $LIBS"

  AS_IF([test "$enable_debug" = no], [
    LDFLAGS="$LDFLAGS -mwindows"
  ])

  AC_DEFINE([USE_WINDOWS], [1], [Define to 1 if building on the windows platform])
])

AS_IF([test "$use_mingw" = yes], [
  AS_IF([test "$enable_dynamic_support_libs" = no], [
    # Static linking on windows, plus a thread library work-around:
    # The --whole-archive switch makes the linker try to static link anyway,
    # even though the application does not call the library.
    LIBS="$LIBS -static-libstdc++ -static-libgcc -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive"
  ])
])

# Defines

# Ensure the prefix is set
test "x$prefix" = xNONE && prefix=$ac_default_prefix

AS_IF([test "$use_mingw" != yes], [
  pkgdatadir='${datadir}/${PACKAGE_TARNAME}'
  EVAL_SHELL_VAR([pkgdatadir])
  AC_DEFINE_UNQUOTED([PACKAGE_DATA_PATH], ["$pkgdatadir"],
		     [Define to the path to the game data])
])

EVAL_SHELL_VAR([localedir])
AC_DEFINE_UNQUOTED([LOCALE_PATH], ["$localedir"], [
  Define to the locale path
])

AS_IF([test "$use_windows" != yes], [
  AC_DEFINE([NO_WINDOWS], [1], [Define to 1 if not using Windows headers])
])

AS_CASE(["$video_backend"],
  [sdl], [
    # Define later
  ], [
    AC_ERROR([Unknown video backend: $video_backend])
])

AS_CASE(["$audio_backend"],
  [openal], [
    AC_DEFINE([USE_OPENAL], [1], [
      Define to 1 if using OpenAL as the audio backend
    ])
  ], [
    AC_ERROR([Unknown audio backend: $audio_backend])
])

AS_CASE(["$input_backend"],
  [sdl], [
    # Define later
  ], [
    AC_ERROR([Unknown input backend: $input_backend])
])

AS_IF([test "$input_backend" = sdl || test "$video_backend" = sdl], [
  AC_DEFINE([USE_SDL], [1],
	    [Define to 1 if using SDL as the video/input backend])
])

AS_IF([test "$enable_curl" = yes], [
  AC_DEFINE([HAVE_LIBCURL], [1],
	    [Define to 1 if using libcurl])
])

AC_DEFINE([NO_MEM_CLASS], [1], [Define to 1 if not monitoring memory alloc])
AS_IF([test "$enable_debug" = yes], [
  AC_DEFINE([DEBUG], [1], [Define to 1 if building with debugging features])
  AC_DEFINE([DYNARRAY_DEBUG_ELEMENT_ACCESS], [1], [
	 Define to 1 if building with element access checking for dynamic arrays
  ])
])

AS_IF([test "$enable_asm" = yes], [
  AC_DEFINE([USE_ASM], [1], [
    Define to 1 if using assembly routines
  ])
])

CXXFLAGS="$CXXFLAGS -I\"\$(top_srcdir)/include\""
CXXFLAGS="$CXXFLAGS -include \"\$(top_builddir)/include/config.h\""

AC_CONFIG_FILES([
  Makefile
  data/Makefile
  include/Makefile
  src/Makefile
  src/curl/Makefile
  src/enet/Makefile
  src/imgfun/Makefile
  src/imgfun/generic/Makefile
  src/imgfun/x86/Makefile
  src/openal/Makefile
  src/platform/Makefile
  po/Makefile.in
])

AC_OUTPUT
echo
AC_MSG_NOTICE([audio backend: $audio_backend])
AC_MSG_NOTICE([video backend: $video_backend])
AC_MSG_NOTICE([input backend: $input_backend])
echo
AC_MSG_NOTICE([x86 asm: $enable_asm])
AC_MSG_NOTICE([debugging features: $enable_debug])
