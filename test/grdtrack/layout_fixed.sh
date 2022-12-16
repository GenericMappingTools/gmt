#!/usr/bin/env bash
ps=layout_fixed.ps
function plot_it {
	gmt grdtrack @Matthews_2016_subduction_subset.txt -Gdummy.nc -C500k/100/100${1} > sz_pol_left.gmt
	awk '{ if ( $1 == ">") print $0 ; else if ($3 <= 0) print $0 }' sz_pol_left.gmt > Lhalfxprofile.gmt
	awk '{ if ( $1 == ">") print $0 ; else if ($3 >= 0) print $0 }' sz_pol_left.gmt > Rhalfxprofile.gmt
	gmt psxy -R -J -O -K -W1p @Matthews_2016_subduction_subset.txt
	gmt psxy -R -J -O Lhalfxprofile.gmt -W1p,red -K
	gmt psxy -R -J Rhalfxprofile.gmt -W1p,blue -O -K
	gmt psxy -R -J Rhalfxprofile.gmt -Sc0.4c -Gblue -O -K
	gmt psxy -R -J Lhalfxprofile.gmt -Sc0.4c -Gred -O -K
	gmt pstext -R -J -O -K sz_pol_left.gmt -F+f9p,Helvetica,white+r0
	gmt psxy -R -J -O -K -Ss0.25i -Gblack @Matthews_2016_subduction_subset.txt
	gmt pstext -R -J -O -K @Matthews_2016_subduction_subset.txt -F+f12p,Helvetica,white+r0
	echo $1 | gmt pstext -R -J -O -K -F+cTL+jTL+f18p -Dj0.2i -Gwhite
}
gmt grdmath -R135/162/42/51 -I1 X = dummy.nc
gmt psbasemap -R135/162/42/51 -JM6i -Bafg -BWSnE -P -K -Xc > $ps
plot_it +f130 >> $ps
gmt psbasemap -R -J -O -Bafg -BWsnE -K -Y3.1i >> $ps
plot_it +f90 >> $ps
gmt psbasemap -R -J -O -Bafg -BWsnE -K -Y3.1i >> $ps
plot_it >> $ps
gmt psxy -R -J -O -T >> $ps
