#!/usr/bin/env bash
#
# Gridding via elastically coupled Green's splines is similar to greenspline gridding
# in that a SVD-based solution allows for the use of a subset of the eigenvalues.
# While a good first step is to use 25% of them, this animation shows the solutions
# for all choices of eigenvalues and tracks the reduction of misfit (both total and
# separately for the east and north components.)  The movie shares the same data and
# setup as one of our test scripts (gpsgridder1.sh) but has been weaponized to do it
# via animation.  Note as we include the contributions from the tiniest eigenvalues
# we rapidly "improve" the misfit while adding spurious variations to the solution.
# The moral is to not try to fit the data exactly.
#
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/Pvvc4vb8G4Y
#
# The script took ~20 minutes to render on a 24-core MacPro 2013, with most of the
# time being used to solve for the cumulative and incremental grid components.

# A few common variables to include
cat << EOF > inc.sh
INC=5m	# Desired grid spacing
DEC=2	# Let grdvector only plot every other node
R=-R122.5W/115W/32.5N/38N	# Area of interest
RATE_CUM=50 # This is in mm/yr or km/Myr - change to use another scalebar
RATE_INC=5 # This is in mm/yr or km/Myr - change to use another scalebar
EOF
# Background script doing the hard work plus making background map
cat << 'EOF' > pre.sh
gmt begin
	# Prepare the GPS data set for use with a 1x1 arc minute grid
	gmt set MAP_VECTOR_SHAPE 0.5
	gmt select @wus_gps_final.txt -R122.5W/115W/32.5N/40N -fg -o0:5 > data.lluv
#	Use blockmean to avoid aliasing
	gmt blockmean $R -I${INC} data.lluv -fg -i0:2,4 -W > blk.llu
	gmt blockmean data.lluv -fg -i0,1,3,5 -W > blk.llv
	gmt convert -A blk.llu blk.llv -o0:2,6,3,7 > blk.lluv
	gmt select blk.lluv $R -fg | awk '{ print($0," 0 ") }' > data.lluvenct
#
#	Do the gridding. There are 2682 data and here we evaluate all possible solutions
	gmt gpsgridder $R -I${INC} -Gtmp.grd blk.lluv -fg -Emisfit.txt -Fd8 -C+c+i -S0.5 -V
#
#	Mask the grids
	cat <<- END > corner.ll
	-114.9 37.
	-114.9 40.1
	-118 40.1
	-115.50 37
	-115.25 32.5
	-114.9 32.5
	-114.9 37
	END
	gmt grdlandmask -Gtmp_mask1.grd -Df
	gmt grdmask -Rtmp_mask1.grd corner.ll -Gtmp_mask2.grd -N1/0/0
	gmt grdmath tmp_mask1.grd tmp_mask2.grd MUL 0 NAN = mask.grd

#	Make a plot of GPS velocity vectors
	gmt coast -Rmask.grd -JM4.2i -Glightgray -Ba1f30m -BWSne -Df -X0.4i -Y0.25i -Wfaint
	gmt plot @CA_fault_data.txt -W0.5p
	gmt velo data.lluvenct -Se.008i/0.95+f8p -A9p -W0.2p,red

#	Place the scale using a geovector of length RATE_CUM
	echo 121.5W 33N 90 ${RATE_CUM}k   | gmt plot -S=0.06i+e+jc -Gblue -W1p,blue
	echo 121.5W 33N ${RATE_CUM} mm/yr | gmt text -F+f8p+jCB -D0/0.07i
#	Map for incremental vectors
	gmt coast -Glightgray -Ba1f30m -BESnw -Df -X4.4i -Wfaint
	gmt plot @CA_fault_data.txt -W0.5p
#	Place the scale using a geovector of length RATE_INC (10x scaling)
	echo 121.5W 33N 90 ${RATE_CUM}k   | gmt plot -S=0.06i+e+jc -Gblue -W1p,blue
	echo 121.5W 33N ${RATE_INC} mm/yr | gmt text -F+f8p+jCB -D0/0.07i
gmt end
EOF
# Main script for the frames
cat << 'EOF' > main.sh
gmt begin
	# Prepare the masked cumulative u,v grids
	gmt grdmath tmp_u_cum_${MOVIE_ITEM}.grd mask.grd MUL = GPS_u.grd
	gmt grdmath tmp_v_cum_${MOVIE_ITEM}.grd mask.grd MUL = GPS_v.grd
	# Plot models
	# Shrink down heads of vectors shorter than 10 km
	gmt grdvector GPS_u.grd GPS_v.grd -Ix${DEC}/${DEC} -JM4.2i -Q0.06i+e+n10 -Gblue -W0.2p,blue -S100i -X0.4i -Y0.25i
	# Prepare the masked incremental grids scaled by 10
	gmt grdmath tmp_u_inc_${MOVIE_ITEM}.grd mask.grd MUL 10 MUL = GPS_du.grd
	gmt grdmath tmp_v_inc_${MOVIE_ITEM}.grd mask.grd MUL 10 MUL = GPS_dv.grd
	gmt grdvector GPS_du.grd GPS_dv.grd -Ix${DEC}/${DEC} -Q0.06i+e+n10 -Gblue -W0.2p,blue -S100i -X4.45i
	gmt plot misfit.txt -i0,4 -qi0:${MOVIE_FRAME} -R0/${MOVIE_NFRAMES}/0/7 -JX8.6i/0.7i -X-4.45i -Y4.0i -B -BWSrt -W0.25p,green
	# Show decrease in rms misfits
	gmt plot misfit.txt -i0,5 -qi0:${MOVIE_FRAME} -W0.25p,blue
	gmt plot misfit.txt -i0,3 -qi0:${MOVIE_FRAME} -W1p,red
gmt end
EOF
# Run the movie
gmt movie main.sh -Iinc.sh -Sbpre.sh -CHD -Nanim15 -H8 -M210,png -Tmisfit.txt -Lf+jTC+t"Cumulative and Incremental Vector Contributions for Eigenvalue %4.4d" \
	-Ls"rms@-u@-"+f12p,Helvetica,green+jTR+o0.5i/0.4i -Ls"rms@-v@-"+f12p,Helvetica,blue+jTR+o0.5i/0.55i -Ls"rms@-c@-"+f12p,Helvetica,red+jTR+o0.5i/0.7i -Fmp4 -V -Zs
