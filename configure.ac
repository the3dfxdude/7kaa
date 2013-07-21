#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([7kaa], [2.14.4], [skfans-devel@lists.sourceforge.net], [],
	[http://7kfans.com/])
AC_CONFIG_SRCDIR([src/client/AM.cpp])
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
    eval $1=$$1
    AS_VAR_IF([prev], [$$1], [
      break
    ])
    AS_VAR_SET([prev], [$$1])
  done
  AS_VAR_POPDEF([prev])
])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_LN_S

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
dnl AC_CHECK_FUNCS([])


# Configuration variables

audio_backend=openal # openal dsound
input_backend=sdl    # sdl dinput none
video_backend=sdl    # sdl ddraw none
net_backend=enet   # enet none
use_windows=no
use_mingw=no

# Set in argument handlers:
# enable_debug=no
# with_directx=no
# with_wine=no
# enable_dynamic_support_libs=no

# Configuration options

AC_ARG_ENABLE([debug],
  AS_HELP_STRING([--enable-debug], [enable debugging features]),
  [], [enable_debug=no]
)

AC_ARG_ENABLE([network],
  AS_HELP_STRING([--disable-network], [disable network support]),
  [], [enable_network=yes]
)

AC_ARG_WITH([directx],
  AS_HELP_STRING([--with-directx], [use DirectX instead of SDL]),
  [], [with_directx=no]
)

AC_ARG_WITH([wine],
  AS_HELP_STRING([--with-wine], [build with winelib using wineg++]),
  [], [with_wine=no]
)

AC_ARG_ENABLE([dynamic-support-libs],
  AS_HELP_STRING([--enable-dynamic-suppport-libs],
		 [do not link statically with libgcc and libstdc++ on Windows]),
  [], [enable_dynamic_support_libs=no]
)


# Select which backends to build

AS_IF([test "$with_wine" = yes], [
  with_directx=yes
])

AS_IF([test "$with_directx" = yes], [
  audio_backend=dsound
  input_backend=dinput
  video_backend=ddraw
  net_backend=none
])

AS_IF([test "$enable_network" = no], [
  net_backend=none
])

AS_IF([test "$audio_backend" = dsound \
       || test "$input_backend" = dinput \
       || test "$video_backend" = ddraw], [
  use_windows=yes
])
AS_CASE(["$host"],
  [*-mingw*], [
    use_windows=yes
    use_mingw=yes
])

AM_CONDITIONAL([AUDIO_OPENAL], [test "$audio_backend" = openal])
AM_CONDITIONAL([AUDIO_DSOUND], [test "$audio_backend" = dsound])
AM_CONDITIONAL([INPUT_DINPUT], [test "$input_backend" = dinput])
AM_CONDITIONAL([INPUT_NONE],   [test "$input_backend" = none])
AM_CONDITIONAL([INPUT_SDL],    [test "$input_backend" = sdl])
AM_CONDITIONAL([NET_NONE],     [test "$net_backend" = none])
AM_CONDITIONAL([NET_ENET],     [test "$net_backend" = enet])
AM_CONDITIONAL([VIDEO_DDRAW],  [test "$video_backend" = ddraw])
AM_CONDITIONAL([VIDEO_NONE],   [test "$video_backend" = none])
AM_CONDITIONAL([VIDEO_SDL],    [test "$video_backend" = sdl])


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


# Library dependencies

AS_IF([test "$audio_backend" = openal], [
  AM_PATH_OPENAL([], [
    AC_ERROR([OpenAL not found])
  ])

  CXXFLAGS="$CXXFLAGS $OPENAL_CFLAGS"
  LIBS="$OPENAL_LIBS $LIBS"
])

AS_IF([test "$input_backend" = sdl \
       || test "$video_backend" = sdl \
       || test "$net_backend" = sdlnet], [
  AM_PATH_SDL([], [], [
    AC_ERROR([SDL not found])
  ])

  CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
  LIBS="$SDL_LIBS $LIBS"
])

AS_IF([test "$net_backend" = sdlnet], [
  SEARCH_LIB_FLAGS([SDLNet_Init], ["-lSDL_net -lwsock32" -lSDL_net], [], [
    AC_ERROR([SDL_net not found])
  ])
])

AS_IF([test "$net_backend" = enet], [
  SEARCH_LIB_FLAGS([enet_initialize], ["-lenet -lwsock32" -lenet], [], [
    AC_ERROR([enet not found])
  ])
])

AS_IF([test "$video_backend" = ddraw], [
  LIBS="-lgdi32 -lddraw -ldxguid $LIBS"
])

AS_IF([test "$audio_backend" = dsound], [
  LIBS="-ldsound $LIBS"
])

AS_IF([test "$input_backend" = dinput], [
  LIBS="-ldinput $LIBS"
])

AS_IF([test "$use_windows" = yes], [
  LIBS="-lole32 -lmsvcrt -lwinmm $LIBS"
  LDFLAGS="-mno-cygwin $LDFLAGS"

  AS_IF([test "$enable_debug" = no], [
    LDFLAGS="$LDFLAGS -mwindows"
  ])
])

AS_IF([test "$use_mingw" = yes], [
  AS_IF([test "$enable_dynamic_support_libs" = no], [
    LIBS="$LIBS -static-libstdc++ -static-libgcc"
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

AS_IF([test "$use_windows" != yes], [
  AC_DEFINE([NO_WINDOWS], [1], [Define to 1 if not using Windows headers])
])

AS_CASE(["$video_backend"],
  [ddraw], [
    AC_DEFINE([USE_DDRAW], [1],
	      [Define to 1 if using DirectDraw as the video backend])
  ],
  [sdl], [
    # Define later
  ],
  [none], [
    AC_DEFINE([USE_NOVIDEO], [1],
	      [Define to 1 if building without a graphical display])
  ], [
    AC_ERROR([Unknown video backend: $video_backend])
])

AS_CASE(["$audio_backend"],
  [dsound], [
    AC_DEFINE([USE_DSOUND], [1], [
      Define to 1 if using DirectSound as the audio backend
    ])
  ],
  [openal], [
    AC_DEFINE([USE_OPENAL], [1], [
      Define to 1 if using OpenAL as the audio backend
    ])
  ], [
    AC_ERROR([Unknown audio backend: $audio_backend])
])

AS_CASE(["$input_backend"],
  [dinput], [
    AC_DEFINE([USE_DINPUT], [1], [
      Define to 1 if using Direct Input as the input backend
    ])
  ],
  [none], [
    AC_DEFINE([USE_NOINPUT], [1], [
      Define to 1 if building without input support
    ])
  ],
  [sdl], [
    # Define later
  ], [
    AC_ERROR([Unknown input backend: $input_backend])
])

AS_CASE(["$net_backend"],
  [none], [
    AC_DEFINE([DISABLE_MULTIPLAYER], [1], [
      Define to 1 to disable multiplayer
    ])
  ],
  [enet], [
    AC_DEFINE([USE_ENET], [1], [
      Define to 1 if using enet as the network backend
    ])
  ], [
    AC_ERROR([Unknown network backend: $net_backend])
])

AS_IF([test "$input_backend" = sdl || test "$video_backend" = sdl], [
  AC_DEFINE([USE_SDL], [1],
	    [Define to 1 if using SDL as the video/input backend])
])

AS_IF([test "$enable_debug" = yes], [
  AC_DEFINE([DEBUG], [1], [Define to 1 if building with debugging features])
])

AC_DEFINE([NO_ASM], [1], [
  Define to 1 if not using assembly routines or if NO_WINDOWS is defined
])

CXXFLAGS="$CXXFLAGS -I\"\$(top_srcdir)/include\""
CXXFLAGS="$CXXFLAGS -include \"\$(top_builddir)/include/config.h\""

AC_CONFIG_FILES([
  Makefile
  data/Makefile
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
  src/multiplayer/Makefile
  src/multiplayer/common/Makefile
  src/multiplayer/sdlnet/Makefile
  src/multiplayer/none/Makefile
  src/server/Makefile
  src/video/Makefile
  src/video/common/Makefile
  src/video/ddraw/Makefile
  src/video/none/Makefile
  src/video/sdl/Makefile
])

AC_OUTPUT
echo
AC_MSG_NOTICE([audio backend: $audio_backend])
AC_MSG_NOTICE([video backend: $video_backend])
AC_MSG_NOTICE([input backend: $input_backend])
AC_MSG_NOTICE([net backend:   $net_backend])
echo
AC_MSG_NOTICE([debugging features: $enable_debug])
