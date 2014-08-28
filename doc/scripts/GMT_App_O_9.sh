#!/bin/bash
#	$Id$
#
#	Makes Fig 9 for Appendix O (labeled lines)
#
R=-R-85/5/10/55
gmt grdgradient topo5.nc -Nt1 -A45 -Gtopo5_int.nc
gmt gmtset FORMAT_GEO_MAP ddd:mm:ssF FONT_ANNOT_PRIMARY +9p FONT_TITLE 22p
gmt project -E-74/41 -C-17/28 -G10 -Q > great_NY_Canaries.txt
gmt project -E-74/41 -C2.33/48.87 -G100 -Q > great_NY_Paris.txt
km=`echo -17 28 | gmt mapproject -G-74/41/k -fg --FORMAT_FLOAT_OUT=%.0f -o2`
cat << EOF > ttt.cpt
0	lightred	3	lightred
3	lightyellow	6	lightyellow
6	lightgreen	100	lightgreen
EOF
gmt grdimage ttt_atl.nc -Itopo5_int.nc -Cttt.cpt $R -JM5.3i -P -K -nc+t1 > GMT_App_O_9.ps
gmt grdcontour ttt_atl.nc -R -J -O -K -C0.5 -A1+u" hour"+v+f8p,Bookman-Demi \
	-GL80W/31N/17W/26N,17W/28N/17W/50N -S2 >> GMT_App_O_9.ps
gmt psxy -R -J -Wfatter,white great_NY_Canaries.txt -O -K  >> GMT_App_O_9.ps
gmt pscoast -R -J -B20f5 -BWSne+t"Tsunami travel times from the Canaries" -N1/thick -O -K \
	-Glightgray -Wfaint -A500 >> GMT_App_O_9.ps
gmt gmtconvert great_NY_*.txt -E | gmt psxy -R -J -O -K -Sa0.15i -Gred -Wthin >> GMT_App_O_9.ps
gmt psxy -R -J -Wthick great_NY_Canaries.txt -O -K \
	-Sqn1:+f8p,Times-Italic+l"Distance Canaries to New York = $km km"+ap+v >> GMT_App_O_9.ps
gmt psxy -R -J great_NY_Paris.txt -O -K -Sc0.08c -Gblack >> GMT_App_O_9.ps
gmt psxy -R -J -Wthinner great_NY_Paris.txt -SqD1000k:+an+o+gblue+LDk+f7p,Helvetica-Bold,white \
	-O -K >> GMT_App_O_9.ps
cat << EOF | gmt pstext -R -J -O -K -Gwhite -Wthin -Dj0.1i/0.1i -F+f8p,Bookman-Demi+j \
	>> GMT_App_O_9.ps
74W	41N	RT	New York
2.33E	48.87N	CT	Paris
17W	28N	CT	Canaries
EOF
gmt psxy -R -J -O -T >> GMT_App_O_9.ps
