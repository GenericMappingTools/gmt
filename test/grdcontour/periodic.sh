#!/usr/bin/env bash
# GMT_KNOWN_FAILURE
# Update 2020/08/11 PW: Currently, the problem with this script is this:
# When plotting with -Zp, the top plot is mostly OK but the labels for 140 and 160
# degrees have contours cutting right through them.  The bottom plot without -Zp
# plots as expected.  I added the debug drawing of the helper lines in red, which
# required the C-code changes to allow a debug pen.
ps=periodic.ps
gmt grdmath -Rg -I1 32 22 SBAZ = t.nc
# Plot as non-periodic.  This plots labels.  The pile of blue contours around
# 180 occurs because the grid goes rapidly from -180 to +180 here and -Z+p is not used
# This is expected and correct.
gmt grdcontour t.nc -JG0/0/5.0i -A20+u@.+d0.75p,red -Bgaf -K -Wa1,blue -Gl-90/0/0/0,0/0/90/0 -P -Xc -Y0.3i > $ps
# Plot as periodic with -Zp.  Transition from -180 to +180 goes well for contouring
# but lon 140, 160 has labels overdrawn by contour. This is the main issue here.
gmt grdcontour t.nc -J -A20+u@.+d0.75p,red -Wa1,blue -Bgaf -Gl-90/0/0/0,0/0/90/0 -Z+p -O -Y5.2i >> $ps
