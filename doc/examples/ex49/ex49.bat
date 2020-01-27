REM               GMT EXAMPLE 49
REM
REM Purpose:      Illustrate data analysis using the seafloor depth/age relationship
REM GMT modules:  blockmode, gmtmath, grdcontour, grdimage, grdsample, makecpt,
REM		basemap, legend, colorbar, plot, xyz2grd
REM

gmt begin ex49
	REM Convert coarser age grid to pixel registration to match bathymetry grid
	gmt grdsample @age_gridline.nc -T -Gage_pixel.nc
	REM Image depths with color-coded age contours
	gmt makecpt -Cabyss -T-7000/0 -H > z.cpt
	gmt makecpt -Chot -T0/100/10 -H > t.cpt
	gmt grdimage @depth_pixel.nc -JM15c -Cz.cpt -B -BWSne --FORMAT_GEO_MAP=dddF
	gmt plot -W1p @ridge_49.txt
	gmt grdcontour age_pixel.nc -A+f14p -Ct.cpt -Wa0.1p+c -GL30W/22S/5E/13S
	gmt colorbar -Cz.cpt -DjTR+w5c/0.4c+h+o0.75c/0.4c -Baf+u" km" -W0.001 -F+p1p+gbeige
	REM Obtain depth, age pairs by dumping grids and pasting results
	gmt grd2xyz age_pixel.nc   -bof > age.bin
	gmt grd2xyz @depth_pixel.nc -bof > depth.bin
	gmt convert -A age.bin depth.bin -bi3f -o2,5,5 -bo3f > depth-age.bin
	REM Create and map density grid of (age,depth) distribution
	gmt xyz2grd -R0/100/-6500/0 -I0.25/25 -r depth-age.bin -bi3f -An -Gdensity.nc
	REM WHy do we need the -R below? otherwise it fails to work
	gmt grdimage density.nc -R0/100/-6500/0 -JX15c/10c -Q -Y12c -Ct.cpt
	REM Obtain modal depths every ~5 Myr
	gmt blockmode -R0/100/-10000/0 -I5/10000 -r -E depth-age.bin -bi3f -o0,2,3 > modal.txt
	REM Compute Parsons & Sclater [1977] depth-age curve
	REM depth(t) =   350 * sqrt(t) + 2500, t < 70 Myr
	REM	   =  6400 - 3200 exp (-t/62.8), t > 70 Myr
	gmt math -T0/100/0.1 T SQRT 350 MUL 2500 ADD T 70 LE MUL 6400 T 62.8 DIV NEG EXP 3200 MUL SUB T 70 GT MUL ADD NEG = ps.txt
	gmt plot ps.txt -W4p,green
	gmt plot ps.txt -W1p
	REM Compute Stein & Stein [1992] depth-age curve
	REM depth(t) =  365 * sqrt(t) + 2600,  t < 20 Myr
	REM	   = 5651 - 2473 * exp (-0.0278*t), t > 20 Myr
	gmt math -T0/100/0.1 T SQRT 365 MUL 2600 ADD T 20 LE MUL 5651 T -0.0278 MUL EXP 2473 MUL SUB T 20 GT MUL ADD NEG = ss.txt
	REM Plot curves and place the legend
	gmt plot ss.txt -W4p,white
	gmt plot ss.txt -W1p
	gmt plot -Ss0.4c -Gblue modal.txt -Ey+p1p,blue
	gmt plot -Ss0.1c -Gwhite modal.txt
	gmt basemap -R0/100/0/6.5 -JX6i/-4i -Bxaf+u" Myr" -Byaf+u" km" -BWsNe
	echo S 0.5c - 0.9c - 4p,green 1c Parsons & Sclater (1977) > tmp.txt
	echo S 0.5c - 0.9c - 4p,white 1c Stein & Stein (1992)	>> tmp.txt
	echo S 0.5c s 0.4c blue - 1c Modal depth estimates		>> tmp.txt
	gmt legend tmp.txt -DjRT+w6.5c+o0.25c -F+p1p+gbeige+s
	echo S 0.5c - 0.9c - 1p 0.75c	> tmp.txt
	echo S 0.5c - 0.9c - 1p 0.75c	>> tmp.txt
	echo S 0.5c s 0.1c white - 0.75c	>> tmp.txt
	gmt legend tmp.txt -DjRT+w6.5c+o0.25c
	del age_pixel.nc age.bin depth.bin depth-age.bin density.nc modal.txt ps.txt ss.txt z.cpt t.cpt
gmt end show
