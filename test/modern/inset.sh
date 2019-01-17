#!/usr/bin/env bash
gmt begin inset ps
	gmt psbasemap -R0/40/20/60 -JM6.5i -Bafg -B+glightgreen
	gmt inset begin -DjTR+w2.5i+o0.2i -F+gpink+p0.5p -M0.1i
		gmt psbasemap -Rg -JA20/20/2.3i -Bafg
		echo INSET | gmt pstext -F+f12p+cTR
	gmt inset end
	echo MAP | gmt pstext -F+f18p+cBL -Dj0.2i
gmt end
