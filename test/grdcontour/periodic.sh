#!/usr/bin/env bash
# Test contouring of phase data with and without -Z+p
ps=periodic.ps
gmt grdmath -Rg -I1 32 22 SBAZ = t.nc
# Plot as non-periodic.  This plots labels.  The pile of blue contours around
# 180 occurs because the grid goes rapidly from -180 to +180 here and -Z+p is not used
# This is expected and correct.
gmt grdcontour t.nc -JG0/0/5.0i -A20+u@.+d0.75p,red -Bgaf -K -Wa1,blue -Gl-90/0/0/0,0/0/90/0 -P -Xc -Y0.3i > $ps
# Plot as periodic with -Z+p.  Transition from -180 to +180 goes well for contouring
gmt grdcontour t.nc -J -A20+u@.+d0.75p,red -Wa1,blue -Bgaf -Gl-90/0/0/0,0/0/90/0 -Z+p -O -Y5.2i >> $ps
