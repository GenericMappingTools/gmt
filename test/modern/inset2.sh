#!/usr/bin/env bash
# Test the alternative inset form where inset begin takes the -R -J to be used in the inset
# and uses it to determine -D.  We plot a coast map in the inset using that size and plot
# a red cross in the middle of the inset and the original map after the inset completes to
# test that we  are back to the original Mercator -R -J settings.
gmt begin inset2 ps
	gmt grdimage @earth_relief_01m -R-48/-43/-26/-20 -JM16c -B -Cworld
	gmt inset begin -DjBR+o0.2c -F+p1p,black -R-80/-28/-43/10 -JM6c
		gmt coast -Wthin -Swhite -Ggray
		gmt plot -A -Gwhite -W0.25p <<- EOF
		-48 -26
		-43 -26
		-43 -20
		-48 -20
		EOF
		echo 45:30W 23S | gmt plot -Sx4p -W0.25p,red
	gmt inset end
	echo 45:30W 23S | gmt plot -Sx12p -W1p,red
gmt end show
