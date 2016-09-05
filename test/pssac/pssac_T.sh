#!/bin/bash
#	$Id$
#
# Description:

PS=pssac_T.ps
SACFILEs="${src:-.}/ntkl.z ${src:-.}/onkl.z"

gmt set PS_MEDIA 21cx32c
gmt pssac $SACFILEs -JX15c/4c -R200/1600/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -P -G+t700/900 > $PS
gmt pssac $SACFILEs -JX15c/4c -R0/1400/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -O -Y5c -T+t-5 -G+t600/800>> $PS
gmt pssac $SACFILEs -JX15c/4c -R-500/500/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -O -Y5c -T+t1 -G+t0/100 >> $PS
gmt pssac $SACFILEs -JX15c/4c -R0/1400/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -O -Y5c -T+r10 -G+t300/400 >> $PS
gmt pssac $SACFILEs -JX15c/4c -R200/1600/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -O -Y5c -T+s300 -G+t1000/1300>> $PS
gmt pssac $SACFILEs -JX15c/4c -R-300/800/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -O -Y5c -T+t1+s100 >> $PS

gmt psxy -J$J -R$R -O -T >> $PS
