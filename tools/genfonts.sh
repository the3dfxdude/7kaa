#!/bin/bash

# this is simple script to generate a new font set
CODESET=8859_2//
FONTSET=88592

./genfont LiberationSerif-Regular.ttf FNT_CASA_$FONTSET.RES casa $CODESET
./genfont LiberationSerif-Regular.ttf FNT_MID_$FONTSET.RES mid $CODESET
./genfont LiberationSans-Bold.ttf FNT_NEWS_$FONTSET.RES news $CODESET
./genfont LiberationSans-Bold.ttf FNT_SAN_$FONTSET.RES san $CODESET
./genfont LiberationSerif-Regular.ttf FNT_SMAL_$FONTSET.RES smal $CODESET
./genfont LiberationSerif-Regular.ttf FNT_STD_$FONTSET.RES std $CODESET
