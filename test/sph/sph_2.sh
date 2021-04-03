#!/usr/bin/env bash
#

ps=sph_2.ps

gmt makecpt -Crainbow -T-9000/9000 > tt.cpt
gmt sphinterpolate lun2.txt -Rg -I1 -Qp -Gtt.nc
gmt grdimage tt.nc -JH0/4.5i -B30g30 -B+t"-Qp" -Ctt.cpt -X0.7i -Y5.5i -K --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p > $ps
gmt sphinterpolate lun2.txt -Rg -I1 -Ql -Gtt.nc
gmt grdimage tt.nc -J -B30g30 -B+t"-Ql" -Ctt.cpt -X5.1i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
gmt sphinterpolate lun2.txt -Rg -I1 -Qg -Gtt.nc
gmt grdimage tt.nc -J -B30g30 -B+t"-Qg" -Ctt.cpt -X-5.1i -Y-5i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
gmt sphinterpolate lun2.txt -Rg -I1 -Qs -Gtt.nc
gmt grdimage tt.nc -J -B30g30 -B+t"-Qs" -Ctt.cpt -X5.1i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
gmt psxy -Rg -J -O lun2.txt -Sc0.02i -G0 -B30g30 -X-2.55i -Y2.5i >> $ps
