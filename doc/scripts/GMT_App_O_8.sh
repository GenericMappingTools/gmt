#!/bin/sh

PS=`basename $0 .sh`
R=-R-85/5/10/55
#ttt atl_topo_15m -E-17/28 -Tttt.b -V
#grdsample ttt.b=1 $R -I5m -Gttt_atl.grd
#grdraster - 2> $$
#ID=`grep ETOPO5 $$ | awk '{print $1}'`
#grdraster $ID $R -Gtopo5.grd
grdgradient topo5.grd -Nt1 -A45 -Gtopo5_int.grd
gmtset PLOT_DEGREE_FORMAT ddd:mm:ssF ANNOT_FONT_SIZE_PRIMARY +9p
project -E74W/41N -C17W/28N -G10 -Q > great_NY_Canaries.d
project -E74W/41N -C2.33/48.87N -G100 -Q > great_NY_Paris.d
km=`echo 17W 28N | mapproject -G74W/41N/k -fg --D_FORMAT=%.0f | cut -f3`
cat << EOF > ttt.cpt
0	lightred	3	lightred
3	lightyellow	6	lightyellow
6	lightgreen	100	lightgreen
EOF
grdimage ttt_atl.grd -Itopo5_int.grd -Cttt.cpt $R -JM6i -P -K > $PS.ps
grdcontour ttt_atl.grd -R -J -O -K -C0.5 -A1+u"hour"+v+s8+f17 -S10 -GL80W/31N/17W/26N,17W/28N/17W/50N >> $PS.ps
psxy -R -J -W7p,white great_NY_Canaries.d -O -K  >> $PS.ps
pscoast -R -J -B20f5:."Tsunami Travel Times from the Canaries":WSne -N1/thick -O -K -Glightgray -Wfaint -A500 >> $PS.ps
gmtconvert great_NY_*.d -E | psxy -R -J -O -K -Sa0.15i -Gred -Wthin >> $PS.ps
psxy -R -J -W1p great_NY_Canaries.d -O -K -Sqn1:+f6+s8+l"Distance Canaries to New York = $km km"+ap+v -W2p >> $PS.ps
psxy -R -J great_NY_Paris.d -O -K -Sc0.08c -Gblack >> $PS.ps
psxy -R -J -W0.5p great_NY_Paris.d -O -K -SqD1000k:+an+o+gblue+kwhite+LDk+s7+f1 >> $PS.ps
cat << EOF | pstext -R -J -O -K -WwhiteOthin -Dj0.1i/0.1i >> $PS.ps
74W	41N	9	0	17	RT	New York
2.33E	48.87N	9	0	17	CT	Paris
17W	28N	9	0	17	CT	Canaries
EOF
psxy -R -J -O /dev/null >> $PS.ps
