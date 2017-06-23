#!/bin/bash
#	$Id$
#
# Description:

R=195/1600/22/27
J=X15c/4c
Bx=x250
By=y1
PS=pssac_M.ps
SACFILEs="ntkl.z onkl.z"

# -Msize
gmt pssac $SACFILEs -J$J -R$R -B$Bx -B$By -BWSen -Ed -M1.5c -K -P > $PS
gmt pssac $SACFILEs -J -R -B$Bx -B$By -BWsen -Ed -M0.8i -K -O -Y5c >> $PS
gmt pssac $SACFILEs -J -R -B$Bx -B$By -BWsen -Ed -M1.5c/-1 -K -O -Y5c >> $PS
gmt pssac $SACFILEs -J -R -B$Bx -B$By -BWsen -Ed -M0.2/0 -K -O -Y5c >> $PS
gmt pssac $SACFILEs -J -R -B$Bx -B$By -BWsen -Ed -M0.002/0.5 -K -O -Y5c >> $PS

gmt psxy -J -R -O -T >> $PS
