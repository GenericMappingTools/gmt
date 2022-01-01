#!/usr/bin/env bash
# Test -A with log axis
ps=flip.ps
gmt makecpt -Cturbo -T0/5000 > color.cpt
gmt pshistogram -R0/5000/0.01/16 -JX16c/10cl -P @bin_data_hist.b -bi1f -Z1 -Ccolor.cpt -F -K -T50 -Wfaint -Bxafg+u" m" -Byafg+l"Frequency (\045)" -BWSne -Xc > $ps
gmt makecpt -Chot -T0/1 -Q > color.cpt
gmt pshistogram -R -J -O @bin_data_hist.b -bi1f -Z1 -Ccolor.cpt+b -F -T50 -Wfaint -Bxafg+u" m" -Byafg+l"Frequency (\045)" -BWSne -A -Y13c >> $ps
