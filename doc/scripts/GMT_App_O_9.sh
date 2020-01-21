#!/usr/bin/env bash
#
#	Makes Fig 9 for Appendix O (labeled lines)
#
gmt begin GMT_App_O_9
R=-R-85/5/10/55
gmt grdgradient @App_O_topo5.nc -Nt1 -A45 -Gtopo5_int.nc
gmt set FORMAT_GEO_MAP ddd:mm:ssF FONT_ANNOT_PRIMARY +9p FONT_TITLE 22p
gmt project -E-74/41 -C-17/28 -G10 -Q > great_NY_Canaries.txt
gmt project -E-74/41 -C2.33/48.87 -G100 -Q > great_NY_Paris.txt
km=`echo -17 28 | gmt mapproject -G-74/41+uk -fg --FORMAT_FLOAT_OUT=%.0f -o2`
gmt makecpt -Clightred,lightyellow,lightgreen -T0,3,6,100 -N
gmt grdimage @App_O_ttt.nc -Itopo5_int.nc -C $R -JM5.3i -nc+t1
gmt grdcontour @App_O_ttt.nc -C0.5 -A1+u" hour"+v+f8p,Bookman-Demi \
	-GL80W/31N/17W/26N,17W/28N/17W/50N -S2
gmt plot -Wfatter,white great_NY_Canaries.txt
gmt coast -B20f5 -BWSne+t"Tsunami travel times from the Canaries" -N1/thick \
	-Glightgray -Wfaint -A500
gmt convert great_NY_*.txt -E | gmt plot $R -Sa0.15i -Gred -Wthin
gmt plot -Wthick great_NY_Canaries.txt \
	-Sqn1:+f8p,Times-Italic+l"Distance Canaries to New York = $km km"+ap+v
gmt plot great_NY_Paris.txt -Sc0.08c -Gblack
gmt plot -Wthinner great_NY_Paris.txt -SqD1000k:+an+o+gblue+LDk+f7p,Helvetica-Bold,white
cat << EOF | gmt text -Gwhite -Wthin -Dj0.1i -F+f8p,Bookman-Demi+j
74W	41N	RT	New York
2.33E	48.87N	CT	Paris
17W	28N	CT	Canaries
EOF
gmt end show
