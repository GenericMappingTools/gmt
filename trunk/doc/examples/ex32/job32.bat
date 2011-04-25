REM		GMT EXAMPLE 32
REM		$Id: job32.bat,v 1.4 2011-04-25 00:21:06 guru Exp $
REM
REM Purpose:	Illustrate draping of an image over topography
REM GMT progs:	grdcut, grdedit, grdgradient, grdreformat, grdtrack, grdview
REM GMT progs:	pscoast, pstext, psxyz
REM DOS calls:	del, echo
REM Credits:	Original by Stephan Eickschen
REM

echo GMT EXAMPLE 32
set ps=..\example_32.ps

REM Here we get and convert the flag of Europe directly from the web through grdreformat using
REM GDAL support. We take into account the dimension of the flag (1000x667 pixels)
REM for a ratio of 3x2.
REM Because GDAL support will not be standard for most unix users, we have stored
REM the result, euflag.nc in this directory.

set Rflag=-R3/9/50/54
REM grdreformat http://upload.wikimedia.org/wikipedia/commons/thumb/b/b7/Flag_of_Europe.svg/1000px-Flag_of_Europe.svg.png=gd euflag.nc=ns
REM grdedit euflag.nc -fg %Rflag%

REM Now get the topography for the same area from GTOPO30 and store it as topo.nc.
REM The DEM file comes from http://eros.usgs.gov/#/Find_Data/Products_and_Data_Available/gtopo30/w020n90
REM We make an gradient grid as well, which we will use to "illuminate" the flag.

REM grdcut W020N90.DEM -Reuflag.nc -Gtopo.nc=ns
grdgradient topo.nc -A0/270 -Gillum.nc -Ne0.6

REM The color map assigns "Reflex Blue" to the lower half of the 0-255 range and "Yellow" to the upper half.
echo 0   0/51/153   127 0/51/153   > euflag.cpt
echo 127 255/204/0  255 255/204/0 >> euflag.cpt

REM The next step is the plotting of the image.
REM We use grdview to plot the topography, euflag.nc to give the color, and illum.nc to give the shading.

set Rplot=%Rflag%/-10/790
grdview topo.nc -JM13c %Rplot% -Ceuflag.cpt -Geuflag.nc -Iillum.nc -Qc -JZ1c -p157.5/30 -P -K -U"Example 32 in Cookbook" > %ps%

REM We now add borders. Because we have a 3-D plot, we want them to be plotted "at elevation".
REM So we write out the borders, pipe them through grdtack and then plot them with psxyz.

pscoast %Rflag% -Df -M -N1 | grdtrack -Gtopo.nc -sa | psxyz %Rplot% -J -JZ -p -W1p,white -O -K >> %ps%

REM Finally, we add dots and names for three cities.
REM Again, grdtrack is used to put the dots "at elevation".

echo 05:41:27 50:51:05 Maastricht  > cities.txt
echo 04:21:00 50:51:00 Bruxelles  >> cities.txt
echo 07:07:03 50:43:09 Bonn       >> cities.txt

grdtrack -Gtopo.nc -sa cities.txt | psxyz -i0,1,3 %Rplot% -J -JZ -p -Sc7p -W1p,white -Gred -K -O >> %ps%
pstext %Rplot% -J -JZ -p -F+f12p,Helvetica-Bold,red+jRM -Dj0.1i/0.0i -O cities.txt >> %ps%

REM Cleanup

del gmt.conf euflag.cpt illum.nc cities.txt
