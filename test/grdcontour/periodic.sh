#!/bin/bash
# Demonstrate issue #536 with labeling of periodic contours
ps=periodic.ps
gmt grdmath -Rg -I1 32 22 SBAZ = t.nc
# Plot as non-periodic.  This plots labels.  The pile of blue contours around
# 180 occurs because the grid goes rapidly from -180 to +180 here and -Zp is not used
# This is expected and correct.
gmt grdcontour t.nc -JG0/0/4.0i -A20+u"\232" -Bgaf -K -Wa1,blue -P -Xc > $ps
# Plot as periodic with -Zp.  Transition from -180 to +180 goes well for contouring
# but we have no contour labels anymore. This is the main issue here.
gmt grdcontour t.nc -J -A20+u"\232" -Wa1,blue -Bgaf -Zp -O -Y5i >> $ps
