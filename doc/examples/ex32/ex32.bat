REM		GMT EXAMPLE 32
REM
REM Purpose:	Illustrate draping of an image over topography
REM GMT modules:	grdcut, grdedit, grdgradient, grdconvert, grdtrack, grdview
REM GMT modules:	coast, text, plot3d, makecpt
REM DOS calls:	echo, del
REM Credits:	Original by Stephan Eickschen
REM
gmt begin ex32
	REM Here we get and convert the flag of Europe directly from the web through grdconvert using
	REM GDAL support. We take into account the dimension of the flag (1000x667 pixels)
	REM for a ratio of 3x2.
	REM To simplify the example, we have stored the result, @euflag.nc in this directory.

	set Rflag=-R3/9/50/54
	REM gmt grdconvert \
	REM   http://upload.wikimedia.org/wikipedia/commons/thumb/b/b7/Flag_of_Europe.svg/1000px-Flag_of_Europe.svg.png=gd \
	REM   euflag.nc=ns
	REM gmt grdedit euflag.nc -fg %Rflag%

	REM Now get the topography for the same area, mask out the oceans and store it as topo_32.nc.

	gmt grdcut @earth_relief_30s_p %Rflag% -Gtopo_32.nc=ns
	gmt grdcut @earth_mask_30s_p %Rflag% -Gmask_32.nc=ns
	gmt grdmath topo_32.nc mask_32.nc 0 GT 0 NAN MUL = topo_32.nc

	REM The color map assigns "Reflex Blue" to the lower half of the 0-255 range and
	REM "Yellow" to the upper half.
	gmt makecpt -C0/51/153,255/204/0 -T0,127,255 -N

	REM The next step is the plotting of the image.
	REM We use gmt grdview to plot the topography, euflag.nc to give the color, and illum.nc to give
	REM the shading.
	set Rplot=%Rflag%/-10/790
	gmt grdview topo_32.nc -JM13c %Rplot% -C -G@euflag.nc -I+a0/270+ne0.6 -Qc -JZ1c -p157.5/30

	REM We now add borders. Because we have a 3-D plot, we want them to be plotted "at elevation".
	REM So we write out the borders, pipe them through grdtrack and then plot them with plot3d.
	gmt coast %Rflag% -Df -M -N1 | gmt grdtrack -Gtopo_32.nc -s+a | gmt plot3d %Rplot% -JZ -p -W1p,white

	REM Finally, we add dots and names for three cities.
	REM Again, gmt grdtrack is used to put the dots "at elevation".
	echo 05:41:27 50:51:05 Maastricht	> cities.txt
	echo 04:21:00 50:51:00 Bruxelles	>> cities.txt
	echo 07:07:03 50:43:09 Bonn			>> cities.txt

	gmt grdtrack -Gtopo_32.nc cities.txt | gmt plot3d %Rplot% -JZ -p -Sc7p -W1p,white -Gred
	gmt text -JZ -p -F+f12p,Helvetica-Bold,red+jRM -Dj0.1i/0 cities.txt

	REM cleanup
	del cities.txt
gmt end show
