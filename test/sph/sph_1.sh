#!/usr/bin/env bash
#

ps=sph_1.ps
mars370=@mars370d.txt

gmt makecpt -Crainbow -T-7000/15000 > tt.cpt
gmt sphinterpolate ${mars370} -Rg -I1 -Qp -Gtt.nc
gmt grdimage tt.nc -JH0/4.5i -B30g30 -B+t"-Qp" -Ctt.cpt -X0.7i -Y5.5i -K --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p > $ps
gmt sphinterpolate ${mars370} -Rg -I1 -Ql -Gtt.nc
gmt grdimage tt.nc -J -B30g30 -B+t"-Ql" -Ctt.cpt -X5.1i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
gmt sphinterpolate ${mars370} -Rg -I1 -Qg -Gtt.nc
gmt grdimage tt.nc -J -B30g30 -B+t"-Qg" -Ctt.cpt -X-5.1i -Y-5i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
gmt sphinterpolate ${mars370} -Rg -I1 -Qs -Gtt.nc
gmt grdimage tt.nc -J -B30g30 -B+t"-Qs" -Ctt.cpt -X5.1i -O -K  --MAP_TITLE_OFFSET=0i --FONT_TITLE=18p >> $ps
gmt psxy -Rg -J -O ${mars370} -Sc0.05i -G0 -B30g30 -X-2.55i -Y2.5i >> $ps
