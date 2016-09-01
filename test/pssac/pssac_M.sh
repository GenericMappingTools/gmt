#!/bin/bash
#	$Id$
#
# Description: 

R=195/1600/22/27
J=X15c/4c
Bx=x250
By=y1
PS=pssac_M.ps

# -Msize
gmt pssac ntkl.z onkl.z -J$J -R$R -B$Bx -B$By -BWSen -Ed -M1.5c -K -P > $PS
gmt pssac ntkl.z onkl.z -J$J -R$R -B$Bx -B$By -BWsen -Ed -M0.8i -K -O -Y5c >> $PS
gmt pssac ntkl.z onkl.z -J$J -R$R -B$Bx -B$By -BWsen -Ed -M1.5c/-1 -K -O -Y5c >> $PS
gmt pssac ntkl.z onkl.z -J$J -R$R -B$Bx -B$By -BWsen -Ed -M0.2/0 -K -O -Y5c >> $PS
gmt pssac ntkl.z onkl.z -J$J -R$R -B$Bx -B$By -BWsen -Ed -M0.002/0.5 -K -O -Y5c >> $PS

gmt psxy -J$J -R$R -O -T >> $PS
