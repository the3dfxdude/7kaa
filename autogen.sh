#!/bin/bash

set -x
rm -rf aclocal.m4 autom4te.cache config.guess config.h.in config.sub configure depcomp install-sh missing
rm -f ABOUT-NLS config.rpath m4/codeset.m4 m4/fcntl-o.m4 m4/gettext.m4 \
      m4/glibc2.m4 m4/glibc21.m4 m4/iconv.m4 m4/intdiv0.m4 m4/intl.m4 \
      m4/intldir.m4 m4/intlmacosx.m4 m4/intmax.m4 m4/inttypes-pri.m4 \
      m4/inttypes_h.m4 m4/lcmessage.m4 m4/lib-ld.m4 m4/lib-link.m4 \
      m4/lib-prefix.m4 m4/lock.m4 m4/longlong.m4 m4/nls.m4 m4/po.m4 \
      m4/printf-posix.m4 m4/progtest.m4 m4/size_max.m4 m4/stdint_h.m4 \
      m4/threadlib.m4 m4/uintmax_t.m4 m4/visibility.m4 m4/wchar_t.m4 \
      m4/wint_t.m4 m4/xsize.m4 po/Makefile.in.in po/Makevars.template \
      po/Rules-quot po/boldquot.sed po/en@boldquot.header po/en@quot.header \
      po/insert-header.sin po/quot.sed po/remove-potcdate.sin
test "$1" == clean && exit 0

autopoint -f

aclocal -I m4
autoheader
autoconf
automake --add-missing

