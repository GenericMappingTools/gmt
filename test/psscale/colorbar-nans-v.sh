#!/usr/bin/env bash
# Show various placements of NaN rectangle on either side of bar
# and with default, none, or user-supplied label.  This tests the
# new +n|N[-|<string>] syntax where +n places it on the left side
# (bottom for vertical bars), +N places NaN box on the right side
# (or top for vertical bars), the - sign means no label, and <string>
# is the user-supplied label [Default is NaN].  So backwards support
# where +n gives a left-side NaN label box.  This relates to PR
# 
# Vertical colorbar tests

gmt begin colorbar-nans-v
	gmt basemap -R0/20/0/21 -Jx1c -B0 -B+t"Vertical Colorbar NaN placement and annotation(s)"+s"Mixed with back- and foreground colors and cyclicity sign" -X2c
	gmt makecpt -Cjet -Ww -H > w.cpt
	gmt colorbar -Cjet -Dx0c/0+jBC+w15c+e+v+n -B -X1.5c -Y3c
	gmt colorbar -Cjet -Dx0c/0+jBC+w15c+e+v+nBAD -B -X2c
	gmt colorbar -Cjet -Dx0c/0+jBC+w15c+e+v+n-+r -B -X2c
	gmt colorbar -Cjet -Dx0c/0+jBC+w15c+e+v+nMissing+r -B -X2c
	gmt colorbar -Cw.cpt -Dx0c/0+jBC+w15c+v+n-+r -B -X2c
	gmt colorbar -Cjet -Dx0c/0+jBC+w15c+e+v+N -B -X2c
	gmt colorbar -Cjet -Dx0c/0+jBC+w15c+e+v+NBAD -B -X2c
	gmt colorbar -Cjet -Dx0c/0+jBC+w15c+e+v+N-+r -B -X2c
	gmt colorbar -Cw.cpt -Dx0c/0+jBC+w15c+v+N!!!+r -B -X2c
gmt end show
