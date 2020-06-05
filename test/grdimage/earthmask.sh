#!/usr/bin/env bash
#
# Plot the earth masks

gmt begin earthmask ps
	gmt makecpt -Clightblue,darkbrown,cyan,lightgreen,white -T-0.5/4.5/1 -N
	gmt grdimage /Users/pwessel/GMTdev/gmtserver-admin/staging/earth/earth_mask/earth_mask_05m_p.grd -JQ0/15c -Baf -BWsNe
	# BUG IN MASK. The next call covered it up for the orig so it will fail until we fix the mask
	#echo -40 -82 | gmt plot -Ss0.3c -Gdarkbrown
	gmt legend -DJBC+w15c+o0/0.5c -F+p1p <<- EOF
	N 5
	S - s 0.5c lightblue 0.25p - Ocean
	S - s 0.5c darkbrown 0.25p - Land
	S - s 0.5c cyan 0.25p - Lakes
	S - s 0.5c lightgreen 0.25p - Islands
	S - s 0.5c white 0.25p - Ponds
	EOF
gmt end show
