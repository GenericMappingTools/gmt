REM		GMT EXAMPLE 11
REM
REM Purpose:	Create a 3-D RGB Cube
REM GMT modules:	set, grdimage, grdmath, text, plot
REM DOS calls: echo, del

gmt begin ex11
	REM Use gmt plot to plot "cut-along-the-dotted" lines.
	gmt set MAP_TICK_LENGTH_PRIMARY 0

	REM First, create grids of ascending X and Y and constant 0.
	REM These are to be used to represent R, G and B values of the darker 3 faces of the cube.
	gmt grdmath -I1 -R0/255/0/255 X = x1.nc
	gmt grdmath Y = y1.nc
	gmt grdmath 0 = c1.nc

	REM Second, create grids of descending X and Y and constant 255.
	REM These are to be used to represent R, G and B values of the lighter 3 faces of the cube.
	gmt grdmath 255 X SUB = x2.nc
	gmt grdmath 255 Y SUB = y2.nc
	gmt grdmath 255       = c2.nc

	gmt plot @cut-here_11.txt -Wthinnest,. -R-51/306/0/1071 -JX3.5i/10.5i -X2.5i -Y0.5i

	gmt set FONT_ANNOT_PRIMARY 12p,Helvetica-Bold

	gmt grdimage x1.nc y1.nc c1.nc -JX2.5i/-2.5i -X0.5i -R0/255/0/255
	gmt plot -Wthinner,white,- @rays_11.txt
	echo 128 128 -45 12p 60\217 > tmp.txt
	echo 102  26 -90 12p 0.4	>> tmp.txt
	echo 204  26 -90 12p 0.8	>> tmp.txt
	echo 10  140 180 16p G		>> tmp.txt
	gmt text tmp.txt --FONT=white -F+a+f
	echo 0 0 0 128 | gmt plot -N -Sv0.15i+s+e -Gwhite -W2p,white

	gmt grdimage x1.nc c1.nc y1.nc -JX2.5i/2.5i -Y2.5i
	gmt plot -Wthinner,white,- @rays_11.txt
	echo 128 128  45 12p 300\217 > tmp.txt
	echo 26  102   0 12p 0.4	>> tmp.txt
	echo 26  204   0 12p 0.8	>> tmp.txt
	echo 140  10 -90 16p R		>> tmp.txt
	echo 100 100 -45 16p V		>> tmp.txt
	gmt text tmp.txt --FONT=white -F+a+f
	echo 0 0 128 0 | gmt plot -N -Sv0.15i+s+e -Gwhite -W2p,white
	echo 0 0 90 90 | gmt plot -N -Sv0.15i+s+e -Gwhite -W2p,white

	gmt grdimage c1.nc x1.nc y1.nc -JX-2.5i/2.5i -X-2.5i
	gmt plot -Wthinner,white,- @rays_11.txt
	echo 128 128 135 12p 180\217 > tmp.txt
	echo 102  26  90 12p 0.4	>> tmp.txt
	echo 204  26  90 12p 0.8	>> tmp.txt
	echo 10  140   0 16p B		>> tmp.txt
	gmt text tmp.txt --FONT=white -F+a+f
	echo 0 0 0 128 | gmt plot -N -Sv0.15i+s+e -Gwhite -W2p,white
	echo 0 0 128 0 | gmt plot -N -Sv0.15i+s+e -Gwhite -W2p,white

	gmt grdimage x2.nc y2.nc c2.nc -JX-2.5i/-2.5i -X2.5i -Y2.5i
	gmt plot -Wthinner,black,- @rays_11.txt
	echo 128 128 225 12p 240\217 > tmp.txt
	echo 102  26 270 12p 0.4	>> tmp.txt
	echo 204  26 270 12p 0.8	>> tmp.txt
	gmt text tmp.txt -F+a+f

	gmt grdimage c2.nc y2.nc x2.nc -JX2.5i/-2.5i -X2.5i
	gmt plot -Wthinner,black,- @rays_11.txt
	echo 128 128 -45 12p 0\217	> tmp.txt
	echo 26  102   0 12p 0.4	>> tmp.txt
	echo 26  204   0 12p 0.8	>> tmp.txt
	echo 100 100  45 16p S		>> tmp.txt
	echo 204  66  90 16p H		>> tmp.txt
	gmt text tmp.txt -F+a+f
	echo 0 0 90 90 | gmt plot -N -Sv0.15i+s+e -Gblack -W2p
	echo 204 204 204 76 | gmt plot -N -Sv0.15i+s+e -Gblack -W2p

	gmt grdimage x2.nc c2.nc y2.nc -JX-2.5i/2.5i -X-2.5i -Y2.5i
	gmt plot -Wthinner,black,- @rays_11.txt
	echo 128 128 135 12p 120\217 > tmp.txt
	echo 26  102 180 12p 0.4	>> tmp.txt
	echo 26  204 180 12p 0.8	>> tmp.txt
	echo 200 200 225 16p GMT	>> tmp.txt
	gmt text tmp.txt -F+a+f
	del *.nc tmp.txt
gmt end show
