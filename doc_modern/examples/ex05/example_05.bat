REM		GMT EXAMPLE 05
REM
REM
REM Purpose:	Generate grid and show monochrome 3-D perspective
REM GMT progs:	grdmath, grdview, pstext
REM DOS calls:	echo, del
REM
echo GMT EXAMPLE 05
set ps=example_05.ps
gmt grdmath -R-15/15/-15/15 -I0.3 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL = sombrero.nc
gmt makecpt -C128 -T-5,5 -N > g.cpt
gmt grdview sombrero.nc -JX6i -JZ2i -B5 -Bz0.5 -BSEwnZ -N-1+gwhite -Qs -I+a225+nt0.75 -X1.5i -Cg.cpt -R-15/15/-15/15/-1/1 -K -p120/30 > %ps%
echo 4.1 5.5 z(r) = cos (2@~p@~r/8) @~\327@~e@+-r/10@+ | gmt pstext -R0/11/0/8.5 -Jx1i -F+f50p,ZapfChancery-MediumItalic+jBC -O >> %ps%
del g.cpt
del sombrero.nc
del .gmt*
