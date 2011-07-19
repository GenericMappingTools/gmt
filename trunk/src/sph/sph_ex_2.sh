#!/bin/sh
#	$Id: sph_ex_2.sh,v 1.4 2011-07-19 21:59:41 guru Exp $
# Example of gridding with sphinterpolate
ps=`basename $0 '.sh'`.ps
makecpt -Crainbow -T-9000/9000/1000 -Z > $$.cpt
sphinterpolate lun2.txt -Rg -I1 -Q0 -G$$.nc
grdimage $$.nc -JH0/4.5i -B30g30:."-Q0": -C$$.cpt -X0.8i -Y5.5i -K --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p > $ps
sphinterpolate lun2.txt -Rg -I1 -Q1 -G$$.nc
grdimage $$.nc -J -B30g30:."-Q1": -C$$.cpt -X4.9i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
sphinterpolate lun2.txt -Rg -I1 -Q2 -G$$.nc
grdimage $$.nc -J -B30g30:."-Q2": -C$$.cpt -X-4.9i -Y-5i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
sphinterpolate lun2.txt -Rg -I1 -Q3 -G$$.nc
grdimage $$.nc -J -B30g30:."-Q3": -C$$.cpt -X4.9i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
psxy -Rg -J -O -K lun2.txt -Sc0.02i -G0 -B30g30 -X-2.45i -Y2.5i >> $ps
psxy -Rg -J -O -T >> $ps
gv $ps &
rm -f *$$*
