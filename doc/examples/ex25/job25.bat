REM
REM             GMT EXAMPLE 25
REM
REM             $Id: job25.bat,v 1.5 2004-09-29 01:29:02 pwessel Exp $
REM
REM Purpose:    Display distribution of antipode types
REM
REM GMT progs:  grdlandmask, grdmath, grd2xyz, gmtmath, grdimage, pscoast, pslegend
REM DOS calls:  del
REM
echo GMT EXAMPLE 25
set master=y
if exist job25.bat set master=n
if %master%==y cd ex25

REM Create D minutes global grid with -1 over oceans and +1 over land
set D=30
grdlandmask -Rg -I%D%m -Dc -A500 -N-1/1/1/1/1 -F -Gwetdry.grd
REM Manipulate so -1 means ocean/ocean antipode, +1 = land/land, and 0 elsewhere
grdmath wetdry.grd DUP 180 ROTX FLIPUD ADD 2 DIV = key.grd
REM Calculate percentage area of each type of antipode match.
grdmath -Rg -I%D%m -F Y COSD 60 %D% DIV 360 MUL DUP MUL PI DIV DIV 100 MUL = scale.grd
grdmath key.grd -1 EQ 0 NAN scale.grd MUL = tmp.grd
grd2xyz tmp.grd -S -ZTLf > key.b
echo -n set ocean= > legend.bat
gmtmath -bi1s -Ca -S key.b SUM UPPER RINT = >> legend.bat
grdmath key.grd 1 EQ 0 NAN scale.grd MUL = tmp.grd
grd2xyz tmp.grd -S -ZTLf > key.b
echo -n set land= >> legend.bat
gmtmath -bi1s -Ca -S key.b SUM UPPER RINT = >> legend.bat
grdmath key.grd 0 EQ 0 NAN scale.grd MUL = tmp.grd
grd2xyz tmp.grd -S -ZTLf > key.b
echo -n set mixed= >> legend.bat
gmtmath -bi1s -Ca -S key.b SUM UPPER RINT = >> legend.bat
REM Generate corresponding color table
echo -1	blue	0	blue > key.cpt
echo 0	gray	1	gray >> key.cpt
echo 1	red	2	red >> key.cpt
REM Create the final plot and overlay coastlines
gmtset ANNOT_FONT_SIZE_PRIMARY +10p PLOT_DEGREE_FORMAT dddF
grdimage key.grd -JKs180/9i -B60/30:."Antipodal comparisons":WsNE -K -Ckey.cpt -Y1.2i -U/-0.75i/-0.95i/"Example 25 in Cookbook" > example_25.ps
pscoast -R -J -O -K -Wthinnest -Dc -A500 >> example_25.ps
REM Place an explanatory legend below
echo N 3 > tmp
echo S 0.15i s 0.2i red  0.25p 0.3i Terrestrial Antipodes [%%land%% %%%%] >> tmp
echo S 0.15i s 0.2i blue 0.25p 0.3i Oceanic Antipodes [%%ocean%% %%%%] >> tmp
echo S 0.15i s 0.2i gray 0.25p 0.3i Mixed Antipodes [%%mixed%% %%%%] >> tmp
echo pslegend -R0/9/0/0.5 -Jx1i/-1i -O -Dx4.5/0/6i/0.3i/TC -Y-0.2i -Fthick tmp > legend.bat
call legend.bat >> example_25.ps
del *.grd
del key.*
del tmp
del legend.bat
del .gmt*
if %master%==y cd ..
