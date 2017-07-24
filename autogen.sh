#!/bin/bash

autoreconf -vif

# Remove files that autopoint brings in, but we don't need.
rm ABOUT-NLS* po/Makevars.template*
