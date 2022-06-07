#!/usr/bin/env bash
# Show the capability of gravprisms to generate the correct prisms
# We start with the grid of a Gaussian seamount (Truncated) and generate three
# prism sets: (1) constant density, (2) variable horizontal density, and (3) full
# variable density.

# 1. Make the seamount, then remove one quadrant:
echo "0 0 30 6" | gmt grdseamount -R-30/30/-30/30 -I1 -r -Ggausssmt.grd -Cg -F0.2 -H7/2400/3030+p0.8 -Wgaussaverho.grd
gmt grdmath gausssmt.grd 1 X 0 LT Y 0 LT MUL SUB MUL = gausssmt.grd
rho_l=$(gmt grdinfo gaussaverho.grd | grep Remark | awk '{print $NF}')
# 2. Make the constant density prisms
gmt gravprisms -Sgausssmt.grd -D${rho_l} -C+wgaussprisms1.txt+q
# 3. Make variable constant prism densities
gmt gravprisms -Sgausssmt.grd -Dgaussaverho.grd -C+wgaussprisms2.txt+q
# 4. Make variable prism densities
gmt gravprisms -Sgausssmt.grd -H7/2400/3030+p0.8 -C+wgaussprisms3.txt+q+z0.2
# Make plots
gmt begin GMT_seamount_prisms
	gmt set FONT_TAG 12p,Times-Italic
 	gmt makecpt -Cturbo -I -T2400/2950
	gmt subplot begin 1x3 -Fs4.8c/2.75c -A
		gmt subplot set 0 -A"@~r@~@-l@-"
		gmt plot3d -R-30/30/-30/30/0/6 -JX5c -JZ1.75c -C -So1q+b gaussprisms1.txt -i0:2,6,3 -Baf -Bzaf -BWSrtZ -p200/20 -Wfaint
		gmt subplot set 1 -A"@~r@~@-l@-(r)"
		gmt plot3d -R-30/30/-30/30/0/6 -JX5c -JZ1.75c -C -So1q+b gaussprisms2.txt -i0:2,6,3 -Baf -BwSrt -p200/20 -Wfaint
		gmt subplot set 2 -A"@~r@~@-l@-(r,z)"
		gmt plot3d -R-30/30/-30/30/0/6 -JX5c -JZ1.75c -C -So1q+b gaussprisms3.txt -i0:2,6,3 -Baf -BwSrt -p200/20 -Wfaint
	gmt subplot end
	gmt colorbar -DJRM+o1.5c/0.5c -Bxaf -By+l"kg\267m@+-3@+"
gmt end show
rm -f gaussprisms?.txt gaussaverho.grd gausssmt.grd
