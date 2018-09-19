#!/bin/bash

PKG_CONFIG_PATH=/c/7kaa-dev/deps/lib/pkgconfig ./configure --disable-silent-rules
make pkgdatadir="" localedir=locale
make install DESTDIR=`pwd`/dest bindir=/ docdir=/ pkgdatadir=/ localedir=/locale
strip dest/7kaa.exe
# Still need to add music, manual and dlls
# then run NSIS on install.nsi
