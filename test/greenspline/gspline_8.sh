#!/usr/bin/env bash
#
# Make and test 2-D Cartesian interpolation using both
# data values and gradients.  We generate input data from
# a Gaussian bump on a plane:
gmt grdmath -R-2/2/-2/2 -I1 0 0 CDIST 2 DIV 2 POW NEG EXP X 0.1 MUL ADD = bump.grd
# Take DDX to get gradients in x-direction
gmt grdmath bump.grd DDX = grads.grd
# High res versions of same grids for plotting
gmt grdmath -R-2/2/-2/2 -I0.1 0 0 CDIST 2 DIV 2 POW NEG EXP X 0.1 MUL ADD = bump1.grd
gmt grdmath bump1.grd DDX = grads1.grd
# Raw values of value and slope at all nodes inside frame (excluding the border)
gmt grd2xyz -R-1/1/-1/1 bump.grd > bump_raw.txt
gmt grd2xyz -R-1/1/-1/1 grads.grd > grads_raw.txt
# Select two gradients and 7 values for final input
awk '{if ($1 == -1 || $2 != 0) print $0}' bump_raw.txt > bump_z.txt
#awk '{if ($2 == 0 && $1 != -1) print $0, 90}' grads_raw.txt > bump_g.txt
# But because DDX is approximate I redid this on a finer grid to get better values:
cat << EOF > bump_g.txt
0	0	0.100000202656 90
1	0	-0.288589894772 90
EOF
# Make the test plot
gmt begin gspline_8 ps
	gmt makecpt -Cjet -T-1.5/1.5	# Keep fixed CPT for all plots
	gmt subplot begin 3x2 -Fs8c -R-2/2/-2/2 -Scb -Srl
	# First show only data (left) and splined result (right) from just using data values
	gmt grdimage bump.grd -c
	gmt plot bump_raw.txt -Sc6p -Gblack
	gmt greenspline bump_raw.txt -R-2/2/-2/2 -I1 -Sc -Gbump_raw.grd -Z1
	gmt grdimage bump_raw.grd -c
	gmt plot bump_raw.txt -Sc6p -Gblack
	# Now grid both the data (left) and the slope samples and display the splined result
	gmt grdimage bump.grd -c
	gmt plot bump_z.txt -Sc6p -Gblack
	gmt plot bump_g.txt -Sc6p -W0.5p
	gmt greenspline bump_z.txt -Abump_g.txt+f2 -R-2/2/-2/2 -I1 -Sc -Ggrads_out.grd -Z1
	gmt greenspline bump_z.txt -Abump_g.txt+f2 -R-2/2/-2/2 -I0.05 -Sc -Ggrads_out1.grd -Z1
	gmt greenspline bump_z.txt -Abump_g.txt+f2 -R-2/2/-2/2 -I0.05 -Sc -Q90 -Ggrad_out1.grd -Z1
	gmt grdimage grads_out.grd -Bxaf -Byafg10 -c
	gmt plot bump_z.txt -Sc6p -Gblack
	gmt plot bump_g.txt -Sc6p -W0.5p
	# Sample the two grids along y = 0 where we have the gradient constraints for x = 0,1
	gmt grdtrack -Gbump1.grd -ELM/RM -o0,2 | gmt plot -R-2/2/-2/2 -W0.5p -Bafg1 -c -l"Without input slope constraints"+jBL
	gmt grdtrack -Ggrads_out1.grd -ELM/RM -o0,2 | gmt plot -W1.5p -l"With input slope constraints"
	awk '{if ($2 == 0) print $1, $3}' bump_z.txt | gmt plot -Sc6p -Gblack -l"Data constraint"
	gmt grdtrack -Gbump1.grd -nn bump_g.txt -o0,4,2 | gmt plot -Sc6p -W0.25p -l"Slope constraint"
	gmt grdtrack -Gbump1.grd -nn bump_g.txt -o0,4,2 | awk '{print $1, $2, 1, $3}' | gmt plot -Sv9p+e+z3 -W0.75p -Gred
	echo "SURFACE ALONG y = 0" | gmt text -F+cTL+f10p -Dj5p -Gwhite -W0.25p
	gmt grdtrack -Ggrads1.grd -ELM/RM -o0,2 | gmt plot -R-2/2/-2/2 -W0.5p -Bafg1 -c -l"Without input slopes"+jBL
	gmt grdtrack -Ggrad_out1.grd -ELM/RM -o0,2 | gmt plot -W1p -l"With input slopes"
	gmt plot bump_g.txt -Sc6p -W0.5p -i0,2 -l"Slope constraint"
	echo "GRADIENT ALONG y = 0" | gmt text -F+cTL+f10p -Dj5p -Gwhite -W0.25p
	gmt subplot end
	gmt colorbar -DJBC
gmt end
