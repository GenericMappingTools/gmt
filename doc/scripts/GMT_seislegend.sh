#!/usr/bin/env bash
#
# Show a basic seismicity legend
#
gmt begin GMT_seislegend ps
	gmt set GMT_THEME cookbook
	gmt set FONT_ANNOT_PRIMARY 10p FONT_TITLE 18p FORMAT_GEO_MAP ddd:mm:ssF

	# Create standard seismicity color table
	gmt makecpt -Cred,green,blue -T0,100,300,10000 -N

	# Create legend input file for NEIS quake plot
	cat > neis.legend <<- END
	H 16p,Helvetica-Bold SEISMICITY AND THE RING OF FIRE
	D 0 0.5p
	N 3
	V 0 1p
	S 0.25c c 0.25c red   0.25p 0.5c Shallow depth (0-100 km)
	S 0.25c c 0.25c green 0.25p 0.5c Intermediate depth (100-300 km)
	S 0.25c c 0.25c blue  0.25p 0.5c Very deep (> 300 km)
	D 0 0.5p
	V 0 0.5p
	N 7
	S 0.25c c 0.15c black - 0.75c M 3
	S 0.25c c 0.20c black - 0.75c M 4
	S 0.25c c 0.25c black - 0.75c M 5
	S 0.25c c 0.30c black - 0.75c M 6
	S 0.25c c 0.35c black - 0.75c M 7
	S 0.25c c 0.40c black - 0.75c M 8
	S 0.25c c 0.45c black - 0.75c M 9
	N 1
	G 9p
	L 12p,Times-Italic RB Data from the US National Earthquake Information Center
	END
	gmt legend -Dx0/0+jBL+o0/1c+w18c -F+p+gfloralwhite+i0.25p+c2p+s neis.legend

	rm neis.legend
gmt end show
