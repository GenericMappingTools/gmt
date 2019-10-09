REM		GMT EXAMPLE 02
REM
REM Purpose:	Make two color images based gridded data
REM GMT modules:	set, grd2cpt, grdimage, makecpt, colorbar, subplot
REM
gmt begin ex02
	gmt set MAP_ANNOT_OBLIQUE 0
	gmt subplot begin 2x1 -A+JTL+o0.1i/0 -Fs6i/3.5i -M0 -R160/20/220/30+r -JOc190/25.5/292/69/6i -X1.5i -Y1.5i -B10 -T"H@#awaiian@# T@#opo and @#G@#eoid@#"
		gmt subplot set 0,0 -Ce1.1i
		gmt grd2cpt @HI_topo_02.nc -Crelief -Z
		gmt grdimage @HI_topo_02.nc -I+a0
		gmt colorbar -DJRM+o0.6i/0+mc -I0.3 -Bx2+lTOPO -By+lkm

		gmt subplot set 1,0 -Ce1.1i
		gmt makecpt -Crainbow -T-2/14/2
		gmt grdimage @HI_geoid_02.nc
		gmt colorbar -DJRM+o0.6i/0+e+mc -Bx2+lGEOID -By+lm
	gmt subplot end
gmt end show
