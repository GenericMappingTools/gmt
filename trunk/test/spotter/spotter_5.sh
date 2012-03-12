#!/bin/bash
#
#       $Id$

. functions.sh
header "Testing grdpmodeler for evaluating APM model"

ps=spotter_5.ps

POLES=$src/WK97.d

# Determine the plate motion rates in effect when the Pacific crust was formed
grdpmodeler $src/pac_age.nc -E$POLES -F$src/Pacific.txt -Gpac_vel.nc -Sr
makecpt -Crainbow -T0/140/5 -Z > t.cpt
grdimage pac_vel.nc -Ct.cpt -JM4.5i -K -Q -X0.75i -Y1.5i > $ps
pscoast -Rpac_vel.nc -J -O -K -Ggray -B30f10WSne >> $ps
echo "130 60 Plate velocity at formation" | pstext -R -J -O -K -T -Dj0.1i -F+jTL+f14p -Gwhite -W1p >> $ps
psscale -Ct.cpt -D2.25i/-0.4i/3.5i/0.15ih -O -K -B50f25/:km/Myr: >> $ps

# Determine how far the crust has moved since formation
grdpmodeler $src/pac_age.nc -E$POLES -F$src/Pacific.txt -Gpac_dist.nc -Sd
makecpt -Crainbow -T0/8000/500 -Z > t.cpt
grdimage pac_dist.nc -Ct.cpt -J -O -K -Q -X4.75i >> $ps
pscoast -Rpac_vel.nc -J -O -K -Ggray -B30f10wSne >> $ps
echo "130 60 Dispacement since formation" | pstext -R -J -O -K -T -Dj0.1i -F+jTL+f14p -Gwhite -W1p >> $ps
psscale -Ct.cpt -D2.25i/-0.4i/3.5i/0.15ih -O -K -B2000f1000/:km: >> $ps
psxy -R -J -O -T >> $ps

pscmp

