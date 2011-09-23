#!/bin/bash

set -x
rm -rf aclocal.m4 autom4te.cache config.guess config.h.in config.sub configure depcomp install-sh missing
test "$1" == clean && exit 0

aclocal -I m4
autoheader
autoconf
automake --add-missing

