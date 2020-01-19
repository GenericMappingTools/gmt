#!/usr/bin/env bash
#               GMT EXAMPLE 49
#
# Purpose:      Illustrate data analysis using the seafloor depth/age relationship
# GMT modules:  blockmode, gmtmath, grdcontour, grdimage, grdsample, makecpt,
#		basemap, legend, colorbar, plot, xyz2grd
#

gmt begin ex49
	# Convert coarser age grid to pixel registration to match bathymetry grid
	gmt grdsample @age_gridline.nc -T -Gage_pixel.nc
	# Image depths with color-coded age contours
	gmt makecpt -Cabyss -T-7000/0 -H > z.cpt
	gmt makecpt -Chot -T0/100/10 -H > t.cpt
	gmt grdimage @depth_pixel.nc -JM6i -Cz.cpt -B -BWSne -X1.5i --FORMAT_GEO_MAP=dddF
	gmt plot -W1p @ridge_49.txt
	gmt grdcontour age_pixel.nc -A+f14p -Ct.cpt -Wa0.1p+c -GL30W/22S/5E/13S
	gmt colorbar -Cz.cpt -DjTR+w2i/0.15i+h+o0.3i/0.15i -Baf+u" km" -W0.001 -F+p1p+gbeige
	# Obtain depth, age pairs by dumping grids and pasting results
	gmt grd2xyz age_pixel.nc -bof > age.bin
	gmt grd2xyz @depth_pixel.nc -bof > depth.bin
	gmt convert -A age.bin depth.bin -bi3f -o2,5,5 -bo3f > depth-age.bin
	# Create and map density grid of (age,depth) distribution
	gmt xyz2grd -R0/100/-6500/0 -I0.25/25 -r depth-age.bin -bi3f -An -Gdensity.nc
	# WHy do we need the -R below? otherwise it fails to work
	gmt grdimage density.nc -R0/100/-6500/0 -JX6i/4i -Q -Y4.8i -Ct.cpt
	# Obtain modal depths every ~5 Myr
	gmt blockmode -R0/100/-10000/0 -I5/10000 -r -E depth-age.bin -bi3f -o0,2,3 > modal.txt
	# Compute Parsons & Sclater [1977] depth-age curve
	# depth(t) =   350 * sqrt(t) + 2500, t < 70 Myr
	#	   =  6400 - 3200 exp (-t/62.8), t > 70 Myr
	gmt math -T0/100/0.1 T SQRT 350 MUL 2500 ADD T 70 LE MUL 6400 T 62.8 DIV NEG EXP 3200 MUL SUB T 70 GT MUL ADD NEG = ps.txt
	gmt plot ps.txt -W4p,green
	gmt plot ps.txt -W1p
	# Compute Stein & Stein [1992] depth-age curve
	# depth(t) =  365 * sqrt(t) + 2600,  t < 20 Myr
	#	   = 5651 - 2473 * exp (-0.0278*t), t > 20 Myr
	gmt math -T0/100/0.1 T SQRT 365 MUL 2600 ADD T 20 LE MUL 5651 T -0.0278 MUL EXP 2473 MUL SUB T 20 GT MUL ADD NEG = ss.txt
	# Plot curves and place the legend
	gmt plot ss.txt -W4p,white
	gmt plot ss.txt -W1p
	gmt plot -Ss0.4c -Gblue modal.txt -Ey+p1p,blue
	gmt plot -Ss0.1c -Gwhite modal.txt
	gmt basemap -R0/100/0/6.5 -JX6i/-4i -Bxaf+u" Myr" -Byaf+u" km" -BWsNe
	gmt legend -DjRT+w2.5i+o0.1i -F+p1p+gbeige+s <<- EOF
	S 0.2i - 0.35i - 4p,green 0.5i Parsons & Sclater (1977)
	S 0.2i - 0.35i - 4p,white 0.5i Stein & Stein (1992)
	S 0.2i s 0.15i blue - 0.5i Modal depth estimates
	EOF
	gmt legend -DjRT+w2.5i+o0.1i <<- EOF
	S 0.2i - 0.35i - 1p 0.3i
	S 0.2i - 0.35i - 1p 0.3i
	S 0.2i s 0.1c white - 0.3i
	EOF
	rm -f age_pixel.nc age.bin depth.bin depth-age.bin density.nc modal.txt ps.txt ss.txt z.cpt t.cpt
gmt end show
