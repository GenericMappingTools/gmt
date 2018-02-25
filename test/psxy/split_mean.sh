#!/bin/bash
# Due to issue #1202. Now passes after fix in r19821
ps=split_mean.ps
gmt set PROJ_LENGTH_UNIT inch

gmt psbasemap -R169.85/170.15/-43.15/-42.85 -JM170.3/-43/6.5i -P -Bf5ma15m -K -V > $ps

gmt makecpt -T-90/90/2.5 > t.cpt

gmt psxy -R -J -O -SW4i -Ct.cpt -K <<END>> $ps
170 -43 55 54 56 #AF
170 -43 -65 -66 -64 #stress
170 -43 25 24 26 #dominant reverse
170 -43 70 69 71 #dominant strike-slip
END

#same as following
gmt psxy -R -J -O -Sw2i -Gblack -K <<END>> $ps
170 -43 53 57 #AF
170 -43 -66 -62 #stress
170 -43 23 27 #dominant reverse
170 -43 68 72 #dominant strike-slip
END

#although should give same azimuths as
gmt psxy -R -J -O -SV0.25c+e -W0.5p -Ct.cpt <<END>> $ps
170 -43 55 55 2.25i #AF
170 -43 -65 -65 2.25i #stress
170 -43 25 25 2.25i #dominant reverse
170 -43 70 70 2.25i #dominant strike-slip
END
