REM
REM             GMT EXAMPLE 25
REM
REM
REM Purpose:    Display distribution of antipode types
REM
REM GMT progs:  gmtset, grdlandmask, grdmath, grd2xyz, gmtmath, grdimage, pscoast, pslegend
REM DOS calls:  del
REM
echo GMT EXAMPLE 25
set ps=example_25.ps

REM Create D minutes global grid with -1 over oceans and +1 over land
set D=30
gmt grdlandmask -Rg -I%D%m -Dc -A500 -N-1/1/1/1/1 -r -Gwetdry.nc
REM Manipulate so -1 means ocean/ocean antipode, +1 = land/land, and 0 elsewhere
gmt grdmath -fg wetdry.nc DUP 180 ROTX FLIPUD ADD 2 DIV = key.nc
REM Calculate percentage area of each type of antipode match.
gmt grdmath -Rg -I%D%m -r Y COSD 60 %D% DIV 360 MUL DUP MUL PI DIV DIV 100 MUL = scale.nc
gmt grdmath -fg key.nc -1 EQ 0 NAN scale.nc MUL = tmp.nc
gmt grd2xyz tmp.nc -s -ZTLa > key.d
echo { printf "set ocean=%%d\n", $1} > awk.txt
gmt math -Ca -S key.d SUM UPPER RINT = | gawk -f awk.txt > script0.bat
gmt grdmath -fg key.nc 1 EQ 0 NAN scale.nc MUL = tmp.nc
gmt grd2xyz tmp.nc -s -ZTLa > key.d
echo { printf "set land=%%d\n", $1} > awk.txt
gmt math -Ca -S key.d SUM UPPER RINT = | gawk -f awk.txt >> script0.bat
gmt grdmath -fg key.nc 0 EQ 0 NAN scale.nc MUL = tmp.nc
gmt grd2xyz tmp.nc -s -ZTLa > key.d
echo { printf "set mixed=%%d\n", $1} > awk.txt
gmt math -Ca -S key.d SUM UPPER RINT = | gawk -f awk.txt >> script0.bat
REM Generate corresponding color table
gmt makecpt -Cblue,gray,red -T-1.5/1.5/1 -N > key.cpt
REM Create the final plot and overlay coastlines
gmt set FONT_ANNOT_PRIMARY +10p FORMAT_GEO_MAP dddF
gmt grdimage key.nc -JKs180/9i -Bx60 -By30 -BWsNE+t"Antipodal comparisons" -K -Ckey.cpt -Y1.2i -nn > %ps%
gmt pscoast -R -J -O -K -Wthinnest -Dc -A500 >> %ps%
REM Place an explanatory legend below
call script0.bat
echo N 3 > tmp
echo S 0.15i s 0.2i red  0.25p 0.3i Terrestrial Antipodes [%land% %%%%] >> tmp
echo S 0.15i s 0.2i blue 0.25p 0.3i Oceanic Antipodes [%ocean% %%%%] >> tmp
echo S 0.15i s 0.2i gray 0.25p 0.3i Mixed Antipodes [%mixed% %%%%] >> tmp
gmt pslegend -R -J -O -DjCB+w6i+jTC -Y-0.2i -F+pthick tmp >> %ps%
del *.nc
del key.*
del tmp
del awk.txt
del script*.bat
del .gmt*
del gmt.conf
