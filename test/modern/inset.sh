#!/bin/bash
gmt begin inset ps
	gmt psbasemap -R0/40/20/60 -JM6.5i -Bafg -B+glightgreen
	gmt inset begin -DjTR+w2.5i+o0.2i -F+gpink+p0.5p -M0.25i
		gmt psbasemap -Rg -JA20/20/2i -Bafg
		echo INSET | gmt pstext -F+f18p+cTR -Dj-0.15i
	gmt inset end
	echo MAP | gmt pstext -F+f18p+cBL -Dj0.2i
gmt end
