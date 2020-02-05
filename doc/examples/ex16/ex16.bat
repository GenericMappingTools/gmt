REM		GMT EXAMPLE 16
REM
REM Purpose:	Illustrates interpolation methods using same data as Example 12.
REM GMT modules:	gmtset, grdview, grdfilter, contour, colorbar, surface, triangulate
REM DOS calls:	del
REM
gmt begin ex16
	gmt set FONT_ANNOT_PRIMARY 9p FONT_TITLE 18p,Times-Roman
	gmt subplot begin 2x2 -M0.1c -Fs8c/0 -R0/6.5/-0.2/6.5 -Jx1c -SCb -SRl+t -Bwesn -Y2i -T"Gridding of Data"
		gmt contour @Table_5_11.txt -C@ex_16.cpt -I -B+t"contour (triangulate)" -c0,0

		gmt surface @Table_5_11.txt -R0/6.5/-0.2/6.5 -I0.2 -Graws0.nc
		gmt grdview raws0.nc -C@ex_16.cpt -Qs -B+t"surface (tension = 0)" -c0,1
		gmt surface @Table_5_11.txt -Graws5.nc -T0.5
		gmt grdview raws5.nc -C@ex_16.cpt -Qs -B+t"surface (tension = 0.5)" -c1,0

		gmt triangulate @Table_5_11.txt -Grawt.nc
		gmt grdfilter rawt.nc -Gfiltered.nc -D0 -Fc1
		gmt grdview filtered.nc -C@ex_16.cpt -Qs -B+t"triangulate @~\256@~ grdfilter" -c1,1
	gmt subplot end
	gmt colorbar -DJBC -C@ex_16.cpt
gmt end show
del raws0.nc raws5.nc rawt.nc filtered.nc
