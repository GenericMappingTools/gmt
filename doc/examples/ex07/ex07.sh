#!/usr/bin/env bash
#		GMT EXAMPLE 07
#
# Purpose:	Make a basemap with earthquakes and isochrons etc
# GMT modules:	coast, legend, text, plot
#
gmt begin ex07
	gmt coast -R-50/0/-10/20 -JM9i -Slightblue -GP26+r300+ftan+bdarkbrown -Dl -Wthinnest -B --FORMAT_GEO_MAP=dddF
	gmt plot @fz_07.txt -Wthinner,-
	gmt plot @quakes_07.txt -h1 -Sci -i0,1,2+s0.01 -Gred -Wthinnest
	gmt plot @isochron_07.txt -Wthin,blue
	gmt plot @ridge_07.txt -Wthicker,orange
	gmt legend -DjTR+w2.2i+o0.2i -F+pthick+ithinner+gwhite --FONT_ANNOT_PRIMARY=18p,Times-Italic <<- EOF
	S 0.1i c 0.08i red thinnest 0.3i ISC Earthquakes
	EOF
	gmt text -F+f30,Helvetica-Bold,white=thin <<- END
	-43 -5 SOUTH
	-43 -8 AMERICA
	 -7 11 AFRICA
	END
gmt end show
