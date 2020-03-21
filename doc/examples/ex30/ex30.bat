REM		GMT EXAMPLE 30
REM
REM Purpose:	Show graph mode and math angles
REM GMT modules:	gmtmath, basemap, text and plot
REM DOS calls:	echo, del
REM
REM Draw generic x-y axes with arrows
gmt begin ex30
	gmt basemap -R0/360/-1.25/1.75 -JX20c/15c -Bx90f30+u@. -By1g10 -BWS+t"Two Trigonometric Functions" --MAP_FRAME_TYPE=graph --MAP_VECTOR_SHAPE=0.5

	REM Draw sine an cosine curves
	gmt math -T0/360/0.1 T COSD = | gmt plot -W3p
	gmt math -T0/360/0.1 T SIND = | gmt plot -W3p,0_6 --PS_LINE_CAP=round

	REM Indicate the x-angle = 120 degrees
	echo 120	-1.25 > lines.txt
	echo 120	1.25 >> lines.txt
	gmt plot lines.txt -W0.5p,-

	echo 360 1 18p,Times-Roman RB x = cos(@%%12%%a@%%%%) > text.txt
	echo 360 0 18p,Times-Roman RB y = sin(@%%12%%a@%%%%) >> text.txt
	echo 120 -1.25 14p,Times-Roman LB 120@.	>> text.txt
	echo 370 -1.35 24p,Symbol LT a			>> text.txt
	echo -5 1.85 24p,Times-Roman RT x,y		>> text.txt
	gmt text text.txt -Dj0.2c -N -F+f+j

	REM Draw a circle and indicate the 0-70 degree angle
	echo 0 0 | gmt plot -R-1/1/-1/1 -Jx3.8c -X9c -Y7c -Sc5c -W1p -N
	echo ^> x-gridline  -Wdefault > points.txt
	echo -1	0	>> points.txt
	echo 1	0	>> points.txt
	echo ^> y-gridline  -Wdefault >> points.txt
	echo 0	-1 >> points.txt
	echo 0	1 >> points.txt
	echo ^> angle = 0 >> points.txt
	echo 0	0 >> points.txt
	echo 1	0 >> points.txt
	echo ^> angle = 120 >> points.txt
	echo 0	0 >> points.txt
	echo -0.5	0.866025 >> points.txt
	echo ^> x-gmt projection -W2p >> points.txt
	echo -0.3333	0 >> points.txt
	echo 0	0 >> points.txt
	echo ^> y-gmt projection -W2p >> points.txt
	echo -0.3333	0.57735 >> points.txt
	echo -0.3333	0 >> points.txt
	gmt plot points.txt -W1p

	echo -0.16666 0 12p,Times-Roman 0 CT x			> tmp.txt
	echo -0.3333 0.2888675 12p,Times-Roman 0 RM y	>> tmp.txt
	echo 0.22 0.27 12p,Symbol -30 CB a				>> tmp.txt
	echo -0.33333 0.6 12p,Times-Roman 30 LB 120@.	>> tmp.txt
	gmt text tmp.txt -Dj0.05i -F+f+a+j

	echo 0 0 1.25c 0 120 | gmt plot -Sm0.4c+e -W1p -Gblack
gmt end show
