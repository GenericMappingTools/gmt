#!/usr/bin/env bash
#
# Gridding via Green's splines allows to the selection of an approximate fit
# by ignoring the smaller eigenvalues.  This way you can trade off misfit
# against model complexity.  However, it is not easy to know how many
# eigenvalues to include.  This animation illustrates gridding of bathymetry data
# (squares) from ship data. We show the earth_relief_01m for comparison in
# the left panel and the gridded result in the center panel as a function of the
# number of eigenvalues included.  The right panel shows the incremental values
# added as new eigenvalues are included.  Note the data distribution is such that
# we have a good coverage in the north with larger gaps in the south - the solution
# quality reflects this disparity.  We also report the RMS misfit between the model
# and the data as the solution builds incrementally. Note that the ship data have
# a few bad tracks which leads to artifacts in the gridded solution.
#
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/7NQa4TORA3E
# The script took ~70 minutes to render on a 24-core MacPro 2013, with most of the
# time being used to solve for the cumulative and incremental grid components.

cat << 'EOF' > pre.sh
gmt begin
	# Convert depth positive down to bathymetry positive up and decimate to a 1m lattice
	gmt convert -Em10 @geologists.txt -i0,1,2+s-1 -s | gmt blockmedian -R158:00W/156:58W/18:00N/19:40N -I1m > geo.txt
	# Create all cumulative and incremental grids, with misfit summary
	gmt greenspline geo.txt -Sc -C+c+i -Ggeo.grd -Z2 -Emisfit.txt
	# Create CPTs and prepare background map with static image and data
	gmt makecpt -Cturbo -T-5000/-1000 -H > z.cpt
	gmt makecpt -Cpolar -T-100/100 -H > dz.cpt
	gmt grdimage -JM8c -X0 -Y0 -R158:00W/156:58W/18:00N/19:40N @earth_relief_01m.grd -Cz.cpt -B0 -Ei -I+d
	gmt grdcontour @earth_relief_01m.grd -C200 -S8
	gmt plot geo.txt -Ss2p -Gblack
	gmt colorbar -Cz.cpt -DjTL+w4c+h+o11p/8p -Bxaf -By+l"km" -F+gwhite+p0.5p -W0.001
gmt end
EOF
cat << 'EOF' > main.sh
gmt begin
	# Plot current solution with contors in the right panel
	gmt grdimage -JM8c -X8c -Y0 geo_cum_${MOVIE_ITEM}.grd -Cz.cpt -B0 -Ei -I+d
	gmt grdcontour geo_cum_${MOVIE_ITEM}.grd -C200 -S8
	gmt plot geo.txt -Ss2p -Cz.cpt
	# Plot the increments in the center panel and add color bar at bottom
	gmt grdimage -X8c geo_inc_${MOVIE_ITEM}.grd -Cdz.cpt -B0 -Ei
	gmt colorbar -Cdz.cpt -DjTR+w4c+h+o19p/8p+e -Bxaf -By+l"m" -F+gwhite+p0.5p
gmt end
EOF
# Run the movie and add two updating labels
gmt movie main.sh -Sbpre.sh -CHD -Nanim05 -Tmisfit.txt -Fmp4 -H8 -Lf+jBR+f12p,Helvetica,white=~2p+t"Eigenvalue %4.4d" -Lc3+jTL+o8.15c/0.15c+gwhite+f12p+p0.25p+t"rms = %6.2lf m" -V -W -Zs
