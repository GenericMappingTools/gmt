#!/usr/bin/env bash
#		GMT EXAMPLE 07
#
# Purpose:	Make a basemap with earthquakes and isochrons etc
# GMT modules:	coast, legend, text, plot
#
gmt begin ex07
	gmt coast -R-50/0/-10/20 -JM24c -Slightblue -GP26+r300+ftan+bdarkbrown -Dl -Wthinnest -B --FORMAT_GEO_MAP=dddF
	gmt plot @fz_07.txt -Wthinner,-
	gmt plot @quakes_07.txt -h1 -Scc -i0,1,2+s0.025 -Gred -Wthinnest -l"ISC Earthquakes"+s0.2c
	gmt plot @isochron_07.txt -Wthin,blue
	gmt plot @ridge_07.txt -Wthicker,orange
	gmt legend -DjTR+o0.5c -F+pthick+ithinner+gwhite --FONT_ANNOT_PRIMARY=18p,Times-Italic
	gmt text -F+f30,Helvetica-Bold,white=thin <<- END
	-43 -5 SOUTH
	-43 -8 AMERICA
	 -7 11 AFRICA
	END
gmt end show
