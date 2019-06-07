#!/usr/bin/env bash
# GMT_KNOWN_FAILURE
# Test that gmtselect can apply a -Z test to data with OGR records,
# where the z comes from the metadata header, and that output it
# will pass the metadata as well by writing an OGR file
# This is related to issue # 624.  There are several problems
# 1) gmtselect -a cannot add text metadata (e.g., names) since writing
#    a dataset (so we get NaNs instead of capitol names)
# 2) We cannot work around that since the output does not retain the
#    original metadata OGR format.
# Thus, the original select.ps was doctored by plotting the text strings
# via awk; these are commented out for reference and so that the script
# will fail until OGR passing is fixed.

ps=select.ps

gmt select -a2=population @capitals.gmt -Z7000000/- > big_tmp.gmt
# Plot all capitals
gmt pscoast -Rg -JN0/6.5i -Gseashell1 -N1/0.25p,darkred -Wfaint -Baf -K -P -A5000 -Dc -Xc > $ps
gmt psxy @capitals.gmt -R -J -W0.25p -Ss0.05i -Ggreen -O -K >> $ps
# Just plot those with > 5 million in red + labels
gmt pscoast -R -J -Gseashell1 -N1/0.25p,darkred -Wfaint -Baf -B+t"World Capitals" -O -K -A5000 -Dc -Y4.75i >> $ps
gmt psxy big_tmp.gmt -R -J -Ss0.1i -W0.25p -Gred -O -K >> $ps
# This line will need further work to specify text via "gmt select -a":
#gmt pstext big_tmp.gmt -R -J -O -K -F+f8p+jCB -Gwhite -Dj0.1i >> $ps
# We made the original select.ps with awk:
#awk -F'\t' '{if ($3 > 7000000) print $1, $2, $4}' @capitals.gmt | sed -e 's/\"//g' | pstext -R -J -O -K -F+f8p+jCB -Gwhite -Dj0.1i >> $ps
gmt pslegend -DjCB+w2.9i+jCT+o0/0.5i -O -K -R -J -F+p1p << EOF >> $ps
S 0.1i s 0.15i red 0.25p 0.3i Capital with over 7 million people
S 0.1i s 0.15i green 0.25p 0.3i Capital with less people than that
EOF
gmt psxy -R -J -O -T >> $ps
