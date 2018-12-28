#!/bin/bash

#PKG_CONFIG_PATH=/c/7kaa-dev/deps/lib/pkgconfig
HOST=i686-w64-mingw32.shared
HOST_OPT=--host=$HOST
./configure --disable-silent-rules $HOST_OPT
make -j6 pkgdatadir="" localedir=locale
make install DESTDIR=`pwd`/dest bindir=/ docdir=/ pkgdatadir=/ localedir=/locale
${HOST}-strip dest/7kaa.exe
# Still need to add music, manual and dlls
# then run NSIS on install.nsi
