#!/bin/bash

autopoint --force
rm ABOUT-NLS* po/Makevars.template*
aclocal --force --install -I m4
autoconf --force
autoheader --force
automake --add-missing --copy --force-missing
