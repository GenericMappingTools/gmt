REM
REM		GMT EXAMPLE 25
REM
REM		$Id: job25.bat,v 1.3 2004-04-28 19:09:24 pwessel Exp $
REM
REM Purpose:	Display distribution of antipode types
REM GMT progs:	grdlandmask, grdmath, grd2xyz, gmtmath, grdimage, pscoast, pslegend
REM DOS calls:	echo
REM
echo GMT EXAMPLE 25
set master=y
if exist job25.bat set master=n
if %master%==y cd ex25
REM Create D minutes global grid with -1 over oceans and +1 over land
set D=30
echo N 3 > key.txt
grdlandmask -Rg -I%D%m -Dc -A500 -N-1/1/1/1/1 -F -Gwetdry.grd
REM Manipulate so -1 means ocean/ocean antipode, +1 = land/land, and 0 elsewhere
grdmath wetdry.grd DUP 180 ROTX FLIPUD ADD 2 DIV = key.grd
REM Calculate percentage area of each type of antipode match.
grdmath -Rg -I%D%m -F Y COSD 60 %D% DIV 360 MUL DUP MUL PI DIV DIV 100 MUL = scale.grd
grdmath key.grd 1 EQ 0 NAN scale.grd MUL = tmp.grd
grd2xyz tmp.grd -S -ZTLf > key.b
gmtmath -bi1s -Ca -S key.b SUM UPPER RINT = | gawk "{printf \"S 0.15i s 0.2i red 0.25p 0.3i Terrestrial Antipodes [%d %%]\n\", $1}" >> key.txt 
grdmath key.grd -1 EQ 0 NAN scale.grd MUL = tmp.grd
grd2xyz tmp.grd -S -ZTLf > key.b
gmtmath -bi1s -Ca -S key.b SUM UPPER RINT = | gawk "{printf \"S 0.15i s 0.2i blue 0.25p 0.3i Oceanic Antipodes [%d %%]\n\", $1}" >> key.txt 
grdmath key.grd 0 EQ 0 NAN scale.grd MUL = tmp.grd
grd2xyz tmp.grd -S -ZTLf > key.b
gmtmath -bi1s -Ca -S key.b SUM UPPER RINT = | gawk "{printf \"S 0.15i s 0.2i gray 0.25p 0.3i Mixed Antipodes [%d %%]\n\", $1}" >> key.txt 
REM Generate corresponding color table
echo -1	blue	0	blue > key.cpt
echo 0		gray	1	gray >> key.cpt
echo 1		red	2	red >> key.cpt
REM Crate the final plot and overlay coastlines
gmtset ANNOT_FONT_SIZE_PRIMARY +10p PLOT_DEGREE_FORMAT dddF
grdimage key.grd -JKs180/9i -B60/30:."Antipodal comparisons":WsNE -K -Ckey.cpt -Y1.2i -U/-0.75i/-0.95i/"Example 25 in Cookbook" > example_25.ps
pscoast -R -J -O -K -Wthinnest -Dc -A500 >> example_25.ps
REM Place an explanatory legend below
pslegend -R0/9/0/0.5 -Jx1i/-1i -O -Dx4.5/0/6i/0.3i/TC -Y-0.2i -Fthick key.txt -S > legend.bat
if %master%==n echo OFF
CALL legend.bat >> example_25.ps
if %master%==n echo ON
del *.grd
del key.*
del .gmt*
if %master%==y cd ..
