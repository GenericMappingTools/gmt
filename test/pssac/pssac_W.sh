#!/bin/bash
#	$Id$
#
# Description:

R=9/20/-2/2
J=X15c/5c
B=1
PS=pssac_W.ps
SACFILEs="${src:-.}"/seis.sac

gmt pssac -R$R -J$J -B$B -W2p,red,- $SACFILEs -K -P > $PS
gmt pssac -R$R -J$J -B$B -W1p $SACFILEs -K -O -Y7c >> $PS
gmt pssac -R$R -J$J -B$B -Wblue $SACFILEs -K -O -Y7c >> $PS
gmt pssac -R$R -J$J -B$B -W. $SACFILEs -K -O -Y7c >> $PS

gmt psxy -R$R -J$J -T -K -O >> $PS
