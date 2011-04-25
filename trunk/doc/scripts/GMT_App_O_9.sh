#!/bin/bash
#	$Id: GMT_App_O_9.sh,v 1.11 2011-04-25 00:21:07 guru Exp $
#
#	Makes Fig 9 for Appendix O (labeled lines)
#
. functions.sh

R=-R-85/5/10/55
grdgradient topo5.nc -Nt1 -A45 -Gtopo5_int.nc
gmtset FORMAT_GEO_MAP ddd:mm:ssF FONT_ANNOT_PRIMARY +9p FONT_TITLE 23p
project -E-74/41 -C-17/28 -G10 -Q > great_NY_Canaries.d
project -E-74/41 -C2.33/48.87 -G100 -Q > great_NY_Paris.d
km=`echo -17 28 | mapproject -G-74/41/k -fg --FORMAT_FLOAT_OUT=%.0f | cut -f3`
cat << EOF > ttt.cpt
0	lightred	3	lightred
3	lightyellow	6	lightyellow
6	lightgreen	100	lightgreen
EOF
grdimage ttt_atl.nc -Itopo5_int.nc -Cttt.cpt $R -JM5.3i -P -K -nc/1 > GMT_App_O_9.ps
grdcontour ttt_atl.nc -R -J -O -K -C0.5 -A1+u"hour"+v+s8+f17 -GL80W/31N/17W/26N,17W/28N/17W/50N \
	-S2 >> GMT_App_O_9.ps
psxy -R -J -Wfatter,white great_NY_Canaries.d -O -K  >> GMT_App_O_9.ps
pscoast -R -J -B20f5:."Tsunami travel times from the Canaries":WSne -N1/thick -O -K -Glightgray \
	-Wfaint -A500 >> GMT_App_O_9.ps
gmtconvert great_NY_*.d -E | psxy -R -J -O -K -Sa0.15i -Gred -Wthin >> GMT_App_O_9.ps
psxy -R -J -Wthick great_NY_Canaries.d -O -K \
	-Sqn1:+f6+s8+l"Distance Canaries to New York = $km km"+ap+v >> GMT_App_O_9.ps
psxy -R -J great_NY_Paris.d -O -K -Sc0.08c -Gblack >> GMT_App_O_9.ps
psxy -R -J -Wthinner great_NY_Paris.d -O -K -SqD1000k:+an+o+gblue+kwhite+LDk+s7+f1 >> GMT_App_O_9.ps
cat << EOF | pstext -R -J -O -K -Gwhite -Wthin -Dj0.1i/0.1i -F+f8p,Bookman-Demi+j >> GMT_App_O_9.ps
74W	41N	RT	New York
2.33E	48.87N	CT	Paris
17W	28N	CT	Canaries
EOF
psxy -R -J -O /dev/null >> GMT_App_O_9.ps
