REM		GMT EXAMPLE 08
REM
REM Purpose:	Make a 3-D bar plot
REM GMT modules:	grd2xyz, makecpt, text, plot3d
REM Unix progs:	echo
REM
gmt begin ex08
	gmt makecpt -Ccubhelix -T-5000/0
	gmt grd2xyz @guinea_bay.nc | gmt plot3d -B -Bz1000+l"Topography (m)" -BWSneZ+b+tETOPO5 -R-0.1/5.1/-0.1/5.1/-5000/0 -JM5i -JZ6i -p200/30 -So0.0833333ub-5000 -Wthinnest -C -i0-2,2
	echo 0.1 4.9 This is the surface of cube | gmt text -JZ -Z0 -F+f24p,Helvetica-Bold+jTL -p
gmt end show
