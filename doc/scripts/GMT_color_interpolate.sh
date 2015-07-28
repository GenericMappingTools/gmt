#!/bin/bash
#	$Id$
#
ps=GMT_color_interpolate.ps
gmt psbasemap -Jx1i -R0/6.8/0/2.0 -B0 -K > $ps

# Plot polar color map in the left; right (top) and wrong (bottom)
gmt makecpt -Cpolar -T-1/1/1 -Z > tmp.cpt
gmt psscale -D1.7/1.6+w3i/0.3i+h+jTC -Ctmp.cpt -B0.5f0.1 -O -K >> $ps
gmt psscale -D1.7/0.7+w3i/0.3i+h+jTC -Ctmp.cpt -B0.5f0.1 -O -K --COLOR_MODEL=hsv >> $ps

# Plot rainbow color map in the left; right (top) and wrong (bottom)
gmt makecpt -Crainbow -T-1/1/2 -Z > tmp.cpt
gmt psscale -D5.1/1.6+w3i/0.3i+h+jTC -Ctmp.cpt -B0.5f0.1 -O -K >> $ps
gmt psscale -D5.1/0.7+w3i/0.3i+h+jTC -Ctmp.cpt -B0.5f0.1 -O -K --COLOR_MODEL=rgb >> $ps

gmt psxy -R -J -Sd0.1i -O -K -Wblack -Gwhite >> $ps <<END
0.2 1.6
1.7 1.6
3.2 1.6
3.6 1.6
6.6 1.6
0.2 0.7
1.7 0.7
3.2 0.7
3.6 0.7
6.6 0.7
END

gmt pstext -R -J -O -F+f14p,Helvetica-Bold+jBC >> $ps <<END
1.7 1.7 polar (RGB)
1.7 0.8 polar (HSV)
5.1 1.7 rainbow (HSV)
5.1 0.8 rainbow (RGB)
END
