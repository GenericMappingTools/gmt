#!/bin/bash
# Test the automatic placement of color bars when selecting a side mid-point
ps=autoscale.ps
gmt makecpt -Cjet -T0/100 > t.cpt
gmt psbasemap -R0/30/0/45 -JM5i -P -Xc -Yc -Baf -Bwsne -K --MAP_FRAME_TYPE=plain > $ps
gmt psscale -R -J -DjBC -Ct.cpt -F+p0.25p,red -Bxaf -By+l"\312C" -O -K >> $ps
gmt psscale -R -J -DJBC -Ct.cpt -F+p0.25p,red -Bxaf -By+l"\312C" -O -K >> $ps
gmt psscale -R -J -DjTC -Ct.cpt -F+p0.25p,red -Bxaf -By+l"\312C" -O -K >> $ps
gmt psscale -R -J -DJTC -Ct.cpt -F+p0.25p,red -Bxaf -By+l"\312C" -O -K >> $ps
gmt psscale -R -J -DjML -Ct.cpt -F+p0.25p,red -Bxaf -By+l"\312C" -O -K >> $ps
gmt psscale -R -J -DJML -Ct.cpt -F+p0.25p,red -Bxaf -By+l"\312C" -O -K >> $ps
gmt psscale -R -J -DjMR -Ct.cpt -F+p0.25p,red -Bxaf -By+l"\312C" -O -K >> $ps
gmt psscale -R -J -DJMR -Ct.cpt -F+p0.25p,red -Bxaf -By+l"\312C" -O >> $ps
