#!/usr/bin/env bash
#
# Description:

ps=pssac_T.ps
SACFILEs="ntkl.z onkl.z"

gmt set PS_MEDIA 21cx38c
gmt pssac $SACFILEs -JX15c/4c -R200/1600/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -P -G+t700/900 -W > $ps
gmt pssac $SACFILEs -J -R0/1400/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -O -Y6c -T+t-5 -G+t600/800 -W >> $ps
gmt pssac $SACFILEs -J -R-500/500/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -O -Y6c -T+t1 -G+t0/100 -W >> $ps
gmt pssac $SACFILEs -J -R0/1400/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -O -Y6c -T+r10 -G+t300/400 -W >> $ps
gmt pssac $SACFILEs -J -R200/1600/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -K -O -Y6c -T+s300 -G+t1000/1300 -W >> $ps
gmt pssac $SACFILEs -J -R-300/800/22/27 -Bx100 -By1 -BWSen -Ed -M1.5c -O -Y6c -T+t1+s100 >> $ps
