#!/bin/bash
#
#       $Id$

ps=spotter_5.ps

POLES=${src}/../../src/spotter/WK97.d

# Determine the plate motion rates in effect when the Pacific crust was formed
gmt grdpmodeler pac_age.nc -E$POLES -FPacific.txt -Gpac_%s.nc -Srd
gmt makecpt -Crainbow -T0/145/5 -Z > t.cpt
gmt grdimage pac_vel.nc -Ct.cpt -JM4.5i -K -Q -X0.75i -Y1.5i > $ps
gmt pscoast -Rpac_vel.nc -J -O -K -Ggray -B30f10 -BWSne >> $ps
echo "130 60 Plate velocity at formation" | gmt pstext -R -J -O -K -T -Dj0.1i -F+jTL+f14p -Gwhite -W1p >> $ps
gmt psscale -Ct.cpt -D2.25i/-0.4i+w3.5i/0.15i+h+jTC -O -K -Bx50f25 -By+lkm/Myr >> $ps

# Determine how far the crust has moved since formation
gmt makecpt -Crainbow -T0/8000/500 -Z > t.cpt
gmt grdimage pac_dist.nc -Ct.cpt -J -O -K -Q -X4.75i >> $ps
gmt pscoast -Rpac_vel.nc -J -O -K -Ggray -B30f10 -BwSne >> $ps
echo "130 60 Dispacement since formation" | gmt pstext -R -J -O -K -T -Dj0.1i -F+jTL+f14p -Gwhite -W1p >> $ps
gmt psscale -Ct.cpt -D2.25i/-0.4i+w3.5i/0.15i+h+jTC -O -K -Bx2000f1000 -By+lkm >> $ps
gmt psxy -R -J -O -T >> $ps
