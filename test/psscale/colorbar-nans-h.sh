#!/usr/bin/env bash
# Show various placements of NaN rectangle on either side of bar
# and with default, none, or user-supplied label.  This tests the
# new +n|N[-|<string>] syntax where +n places it on the left side
# (bottom for vertical bars), +N places NaN box on the right side
# (or top for vertical bars), the - sign means no label, and <string>
# is the user-supplied label [Default is NaN].  So backwards support
# where +n gives a left-side NaN label box.  This relates to PR
#
# Horizontal colorbar tests


gmt begin colorbar-nans-h
	gmt basemap -R0/21/0/24 -Jx1c -B0 -B+t"Horizontal colorbar NaN placement and annotation(s)"+s"Mixed with back- and foreground colors and cyclicity sign" -X2c
	gmt makecpt -Cjet -Ww -H > w.cpt
	gmt colorbar -Cjet -Dx10c/0+jBC+w15c+e+h+n -B -Y1.5c -X1c
	gmt colorbar -Cjet -Dx10c/0+jBC+w15c+e+h+nBAD -B -Y2.5c
	gmt colorbar -Cjet -Dx10c/0+jBC+w15c+e+h+n-+r -B -Y2.5c
	gmt colorbar -Cjet -Dx10c/0+jBC+w15c+e+h+nMissing+r -B -Y2.5c
	gmt colorbar -Cw.cpt -Dx10c/0+jBC+w15c+h+n-+r -B -Y2.5c
	gmt colorbar -Cjet -Dx10c/0+jBC+w15c+e+h+N -B -Y2.5c
	gmt colorbar -Cjet -Dx10c/0+jBC+w15c+e+h+NBAD -B -Y2.5c
	gmt colorbar -Cjet -Dx10c/0+jBC+w15c+e+h+N-+r -B -Y2.5c
	gmt colorbar -Cw.cpt -Dx10c/0+jBC+w15c+h+N!!!+r -B -Y2.5c
gmt end show
