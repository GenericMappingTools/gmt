REM
REM             GMT EXAMPLE 25
REM
REM             $Id: job25.bat,v 1.13 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:    Display distribution of antipode types
REM
REM GMT progs:  gmtset, grdlandmask, grdmath, grd2xyz, gmtmath, grdimage, pscoast, pslegend
REM DOS calls:  del
REM
echo GMT EXAMPLE 25
set master=y
if exist job25.bat set master=n
if %master%==y cd ex25

REM Create D minutes global grid with -1 over oceans and +1 over land
set D=30
grdlandmask -Rg -I%D%m -Dc -A500 -N-1/1/1/1/1 -F -Gwetdry.nc
REM Manipulate so -1 means ocean/ocean antipode, +1 = land/land, and 0 elsewhere
grdmath wetdry.nc DUP 180 ROTX FLIPUD ADD 2 DIV = key.nc
REM Calculate percentage area of each type of antipode match.
grdmath -Rg -I%D%m -F Y COSD 60 %D% DIV 360 MUL DUP MUL PI DIV DIV 100 MUL = scale.nc
grdmath key.nc -1 EQ 0 NAN scale.nc MUL = tmp.nc
grd2xyz tmp.nc -S -ZTLa > key.d
echo { printf "set ocean=%%d\n", $1} > awk.txt
gmtmath -Ca -S key.d SUM UPPER RINT = | gawk -f awk.txt > script0.bat
grdmath key.nc 1 EQ 0 NAN scale.nc MUL = tmp.nc
grd2xyz tmp.nc -S -ZTLa > key.d
echo { printf "set land=%%d\n", $1} > awk.txt
gmtmath -Ca -S key.d SUM UPPER RINT = | gawk -f awk.txt >> script0.bat
grdmath key.nc 0 EQ 0 NAN scale.nc MUL = tmp.nc
grd2xyz tmp.nc -S -ZTLa > key.d
echo { printf "set mixed=%%d\n", $1} > awk.txt
gmtmath -Ca -S key.d SUM UPPER RINT = | gawk -f awk.txt >> script0.bat
REM Generate corresponding color table
echo -1.5	blue	-0.5	blue > key.cpt
echo -0.5	gray	0.5	gray >> key.cpt
echo 0.5	red	1.5	red >> key.cpt
REM Create the final plot and overlay coastlines
gmtset ANNOT_FONT_SIZE_PRIMARY +10p PLOT_DEGREE_FORMAT dddF
grdimage key.nc -Sn -JKs180/9i -B60/30:."Antipodal comparisons":WsNE -K -Ckey.cpt -Y1.2i -U/-0.75i/-0.95i/"Example 25 in Cookbook" > ..\example_25.ps
pscoast -R -J -O -K -Wthinnest -Dc -A500 >> ..\example_25.ps
REM Place an explanatory legend below
if %master%==n echo off
call script0.bat
echo N 3 > tmp
echo S 0.15i s 0.2i red  0.25p 0.3i Terrestrial Antipodes [%land% %%%%] >> tmp
echo S 0.15i s 0.2i blue 0.25p 0.3i Oceanic Antipodes [%ocean% %%%%] >> tmp
echo S 0.15i s 0.2i gray 0.25p 0.3i Mixed Antipodes [%mixed% %%%%] >> tmp
echo pslegend -R0/9/0/0.5 -Jx1i/-1i -O -Dx4.5i/0/6i/0.3i/TC -Y-0.2i -Fthick tmp -Sscript2.bat > script1.bat
call script1.bat
call script2.bat >> ..\example_25.ps
if %master%==n echo on
del *.nc
del key.*
del tmp
del awk.txt
del script*.bat
del .gmt*
if %master%==y cd ..
