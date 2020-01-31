REM       GMT EXAMPLE 29
REM
REM Purpose:  Illustrates spherical surface gridding with Green's function of splines
REM GMT modules:  makecpt, grdcontour, grdimage, grdmath greenspline, colorbar, text
REM DOS calls:	echo, del
REM
gmt begin ex29
	REM This example uses 370 radio occultation data for Mars to grid the topography.
	REM Data and information from Smith, D. E., and M. T. Zuber (1996), The shape of
	REM Mars and the topographic signature of the hemispheric dichotomy, Science, 271, 184-187.

	REM Make Mars PROJ_ELLIPSOID given their three best-fitting axes:
	set a=3399.472
	set b=3394.329
	set c=3376.502
	gmt grdmath -Rg -I4 -r X COSD %a% DIV DUP MUL X SIND %b% DIV DUP MUL ADD Y COSD DUP MUL MUL Y SIND %c% DIV DUP MUL ADD SQRT INV = PROJ_ELLIPSOID.nc

	REM  Do both Parker and Wessel/Becker solutions (tension = 0.9975)
	gmt greenspline -RPROJ_ELLIPSOID.nc @mars370.txt -D4 -Sp -Gmars.nc
	gmt greenspline -RPROJ_ELLIPSOID.nc @mars370.txt -D4 -Sq0.9975 -Gmars2.nc
	REM Scale to km and remove PROJ_ELLIPSOID
	gmt grdmath mars.nc 1000 DIV PROJ_ELLIPSOID.nc SUB = mars.nc
	gmt grdmath mars2.nc 1000 DIV PROJ_ELLIPSOID.nc SUB = mars2.nc

	gmt set FONT_TAG 14p,Helvetica-Bold
	gmt subplot begin 2x1 -Fs18c/0 -Rg -JH0/18c -B30g30 -BWsne -A -M0.6c
		gmt subplot set 0
		gmt grdimage mars.nc -I+ne0.75+a45 --FONT_ANNOT_PRIMARY=12p
		gmt grdcontour mars.nc -C1 -A5 -Glz+/z-
		gmt plot -Sc0.1c -Gblack @mars370.txt
		gmt colorbar -DJBC+o0/0.4c+w6i/0.25c -I --FONT_ANNOT_PRIMARY=12p -Bx2f1 -By+lkm

		gmt subplot set 1
		gmt makecpt -Crainbow -T-7/15
		gmt grdimage mars2.nc -I+ne0.75+a45 --FONT_ANNOT_PRIMARY=12p
		gmt grdcontour mars2.nc -C1 -A5 -Glz+/z-
		gmt plot -Sc0.1c -Gblack @mars370.txt
	gmt subplot end
	del mars.nc mars2.nc PROJ_ELLIPSOID.nc
gmt end show
