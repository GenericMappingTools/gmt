#!/usr/bin/env bash
# Testing gmt trend2d

ps=trend.ps

gmt makecpt -Crainbow -T690/960/10 > z.cpt
gmt makecpt -Cjet -T-60/120/10 > r.cpt
gmt makecpt -Chot -T0.7/1/0.02 > w.cpt
gmt pscontour -R0/6.5/0/6.5 @Table_5_11.txt -C25 -A50 -JX3i -Y6.5i -Baf -B+tData -K -X1i -I -Cz.cpt -P > $ps
gmt triangulate -M @Table_5_11.txt | gmt psxy -R -J -O -K -W0.25p,- >> $ps
gmt psxy -R -J -O -K @Table_5_11.txt -Sc0.1c -Gblack >> $ps
gmt psscale -Cz.cpt -Dx1.5i/-0.5i+w3i/0.1i+h+jTC -O -K -Ba >> $ps
gmt trend2d @Table_5_11.txt -Fxyrmw -N3r > trend.txt
gmt pscontour -R trend.txt -Cr.cpt -J -Baf -B+tRedisual -I -O -K -X3.5i -i0:2 >> $ps
gmt psscale -Cr.cpt -Dx1.5i/-0.5i+w3i/0.1i+h+jTC -O -K -Ba >> $ps
gmt pscontour -R trend.txt -Cz.cpt -J -Baf -B+tTrend -I -O -K -X-3.5i -Y-5i -i0,1,3 >> $ps
gmt psscale -Cz.cpt -Dx1.5i/-0.5i+w3i/0.1i+h+jTC -O -K -Ba >> $ps
gmt pscontour -R trend.txt -Cw.cpt -J -Baf -B+tWeights -I -O -K -X3.5i -i0,1,4 >> $ps
gmt psscale -Cw.cpt -Dx1.5i/-0.5i+w3i/0.1i+h+jTC -O -K -Ba >> $ps
gmt psxy -R -J -O -T >> $ps

