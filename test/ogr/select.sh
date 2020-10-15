#!/usr/bin/env bash
# Test that gmtselect can apply a -Z test to data with OGR records,
# where the z comes from the metadata header, and we also want text

ps=select.ps

gmt select -a2=population,3=name @capitals.gmt -Z7000000/- -o0,1,t > big_tmp.gmt
# Plot all capitals
gmt pscoast -Rg -JN0/6.5i -Gseashell1 -N1/0.25p,darkred -Wfaint -Baf -K -P -A5000 -Dc -Xc > $ps
gmt psxy @capitals.gmt -R -J -W0.25p -Ss0.05i -Ggreen -O -K >> $ps
# Just plot those with > 5 million in red + labels
gmt pscoast -R -J -Gseashell1 -N1/0.25p,darkred -Wfaint -Baf -B+t"World Capitals" -O -K -A5000 -Dc -Y4.75i >> $ps
gmt psxy big_tmp.gmt -R -J -Ss0.1i -W0.25p -Gred -O -K >> $ps
gmt pstext -R -J -O -K -F+f8p+jCB -Gwhite -Dj0.1i big_tmp.gmt >> $ps
gmt pslegend -DjCB+w2.9i+jCT+o0/0.5i -O -R -J -F+p1p << EOF >> $ps
S 0.1i s 0.15i red 0.25p 0.3i Capital with over 7 million people
S 0.1i s 0.15i green 0.25p 0.3i Capital with less people than that
EOF
