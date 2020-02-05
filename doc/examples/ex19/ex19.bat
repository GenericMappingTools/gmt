REM		GMT EXAMPLE 19
REM
REM Purpose:	Illustrates various color pattern effects for maps
REM GMT modules:	grdimage, grdmath, makecpt, coast, text, image, makecpt
REM DOS calls: echo, rm
gmt begin ex19
	gmt grdmath -Rd -I1 -r Y COSD 2 POW = lat.nc
	gmt grdmath X = lon.nc
	gmt makecpt -Cwhite,blue -T0,1 -Z -N -H > lat.cpt
	gmt makecpt -Crainbow -T-180/180 -H > lon.cpt
	gmt subplot begin 3x1 -Fs16c/0 -M0 -Bbltr -Rd -JI0/16c
		REM   First make a worldmap with graded blue oceans and rainbow continents
		gmt grdimage lat.nc -Clat.cpt -nl -c0,0
		gmt coast -Dc -A5000 -Gc
		gmt grdimage lon.nc -Clon.cpt -nl
		gmt coast -Q
		gmt coast -Dc -A5000 -Wthinnest
		echo 0 20 16TH INTERNATIONAL | gmt text -F+f32p,Helvetica-Bold,red=thinner
		echo 0 -10 GMT CONFERENCE | gmt text -F+f32p,Helvetica-Bold,red=thinner
		echo 0 -30 Honolulu, Hawaii, April 1, 2020 | gmt text -F+f18p,Helvetica-Bold,green=thinnest
		REM   Then show example of color patterns and placing a PostScript image
		gmt coast -Dc -A5000 -Gp86+fred+byellow+r100 -Sp@circuit.png+r100 -c1,0
		echo 0 30 SILLY USES OF | gmt text -F+f32p,Helvetica-Bold,lightgreen=thinner
		echo 0 -30 COLOR PATTERNS | gmt text -F+f32p,Helvetica-Bold,magenta=thinner
		gmt image -DjCM+w7.5c @GMT_covertext.eps
		REM   Finally repeat 1st plot but exchange the colors
		gmt grdimage lon.nc -Clon.cpt -nl -c2,0
		gmt coast -Dc -A5000 -Gc
		gmt grdimage lat.nc -Clat.cpt -nl
		gmt coast -Q
		gmt coast -Dc -A5000 -Wthinnest
		echo 0 20 16TH INTERNATIONAL | gmt text -F+f32p,Helvetica-Bold,red=thinner
		echo 0 -10 GMT CONFERENCE | gmt text -F+f32p,Helvetica-Bold,red=thinner
		echo 0 -30 Honolulu, Hawaii, April 1, 2020 | gmt text -F+f18p,Helvetica-Bold,green=thinnest
	gmt subplot end
gmt end show
del lat.nc lon.nc lat.cpt lon.cpt
