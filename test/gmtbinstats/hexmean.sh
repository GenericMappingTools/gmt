#!/usr/bin/env bash
# Test unweighted hexagon tile binning in gmtbinstats
#
ps=hexmean.ps
gmt binstats hex_data.txt -R0/5/0/3 -I1 -Th -Ca > ave.txt
gmt binstats hex_data.txt -R0/5/0/3 -I1 -Th -Cs > std.txt
gmt makecpt -Cjet -E ave.txt > ave.cpt
gmt makecpt -Chot -E std.txt > std.cpt
R=$(gmt info ave.txt -Ie)
gmt psxy std.txt $R -Jx3c -P -Baf -BWNse -Cstd.cpt -Sh3.46410161514c -W1p -K -Y3c > $ps
gmt psxy -R -J -O -K hex_data.txt -Sc0.1c -Gblack >> $ps
gmt psscale -Cstd.cpt -R -J -DJBC -Bxaf -By+lstdev -O -K >> $ps
gmt psxy ave.txt -R -J -O -K -Baf -BWNse -Cave.cpt -Sh3.46410161514c -W1p -Y13c >> $ps
gmt psxy -R -J -O -K hex_data.txt -Sc0.1c -Gblack >> $ps
gmt psscale -Cave.cpt -R -J -DJBC -Bxaf -By+lMean -O >> $ps
