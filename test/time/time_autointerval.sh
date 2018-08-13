#!/bin/bash
# Test various time ranges for auto-annotations
ps=time_autointerval.ps
gmt psbasemap -Vl -R2008T/2016T/0/1 -JX6iT/1i -Baf -BlStr -P -K -Xc -Y0.75i > $ps
echo 2008T/2016T | gmt pstext -R -J -O -K -F+f14p+cCM >> $ps
gmt psbasemap -Vl -R2008T/2009T/0/1 -J -Baf -BlStr -O -K -Y1.45i >> $ps
echo 2008T/2009T | gmt pstext -R -J -O -K -F+f14p+cCM >> $ps
gmt psbasemap -Vl -R2008-03-01T/2008-08-05T/0/1 -J -Baf -BlStr -O -K -Y1.45i >> $ps
echo 2008-03-01T/2008-07-05T | gmt pstext -R -J -O -K -F+f14p+cCM >> $ps
gmt psbasemap -Vl -R2008-03-01T/2008-04-05T/0/1 -J -Baf -BlStr -O -K -Y1.45i >> $ps
echo 2008-03-01T/2008-04-05T | gmt pstext -R -J -O -K -F+f14p+cCM >> $ps
gmt psbasemap -Vl -R2008-03-01T/2008-03-03T/0/1 -J -Baf -BlStr -O -K -Y1.45i >> $ps
echo 2008-03-01T/2008-03-03T | gmt pstext -R -J -O -K -F+f14p+cCM >> $ps
gmt psbasemap -Vl -R2008-03-01T09:00/2008-03-01T16:00/0/1 -J -Baf -BlStr -O -K -Y1.45i >> $ps
echo 2008-03-01T09:00/2008-03-01T16:00 | gmt pstext -R -J -O -K -F+f14p+cCM >> $ps
gmt psbasemap -Vl -R2008-03-01T09:00/2008-03-01T10:20/0/1 -J -Baf -BlStr -O -K -Y1.45i >> $ps
echo 2008-03-01T09:00/2008-03-01T10:20 | gmt pstext -R -J -O -F+f14p+cCM >> $ps
