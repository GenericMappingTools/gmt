REM		GMT EXAMPLE 13
REM
REM Purpose:	Illustrate vectors and contouring
REM GMT modules:	set, grdmath, grdcontour, grdvector, subplot
REM DOS calls:	del
REM
gmt begin ex13
	gmt set FONT_HEADING 40p,Times-Italic
	gmt grdmath -R-2/2/-2/2 -I0.1 X Y R2 NEG EXP X MUL = z.nc
	gmt grdmath z.nc DDX = dzdx.nc
	gmt grdmath z.nc DDY = dzdy.nc
	gmt subplot begin 2x2 -M0.1c -Ff15c -BWSne -T"z(x,y) = x@~\327@~exp(-x@+2@+-y@+2@+)"
		gmt grdcontour z.nc -C0.05 -A0.1 -Gd5c -S4 -T+d8p/2p -c0,0
		gmt grdcontour z.nc -C0.05 -Gd5c -S4 -c0,1
		gmt grdvector dzdx.nc dzdy.nc -I0.2 -Q0.25c+e+n0.25i+h0.5 -Gblack -W1p -S12c
		gmt grdcontour dzdx.nc -C0.10 -A0.5 -Gd5c -S4 -T+d8p/2p -c1,0
		gmt grdcontour dzdy.nc -C0.05 -A0.2 -Gd5c -S4 -T+d8p/2p -c1,1
	gmt subplot end
gmt end show
del z.nc dzdx.nc dzdy.nc
