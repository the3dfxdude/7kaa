#!/bin/bash

autopoint --force > /dev/null
rm ABOUT-NLS* po/Makevars.template*
aclocal --force --install -I m4
autoconf --force
autoheader --force
automake --add-missing --copy --force-missing
