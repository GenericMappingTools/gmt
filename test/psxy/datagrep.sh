#!/bin/bash
#	$Id$
# Test that psxy can select on data records using -e
ps=datagrep.ps
cat << EOF > no_HI_LV.txt
Hawaii-Emperor
Louisville
EOF
gmt pscoast -R140E/280E/60S/60N -JM7i -P -Baf -B+t"Dated seamounts in the Pacific" -Ggray -K -Xc > $ps
# Plot all but HI and LV
gmt psxy @Pacific_Ages.txt -R -J -O -K -e~+fno_HI_LV.txt -St0.1i -Ggreen >> $ps
# Plot just HI
gmt psxy @Pacific_Ages.txt -R -J -O -K -eHawaii-Emperor -St0.1i -Gred >> $ps
# Plot just LV
gmt psxy @Pacific_Ages.txt -R -J -O -eLouisville -St0.1i -Gblue >> $ps
