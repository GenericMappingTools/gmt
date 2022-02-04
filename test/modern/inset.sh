#!/usr/bin/env bash
gmt begin inset ps
	gmt basemap -R0/40/20/60 -JM6.5i -Bafg -B+glightgreen
	gmt inset begin -DjTR+w2.5i+o0.2i -F+gpink+p0.5p -M0.1i
		gmt basemap -Rg -JA20/20/2.3i -Bafg
		gmt text -F+f12p+cTR+tINSET
	gmt inset end
	gmt text -F+f18p+cBL+tMAP -Dj0.2i
gmt end show
