#!/usr/bin/env bash
# Test # 1 in Sandwell & Wessel tarball test data sets but with weights
# Due to hairline differences in many gridlines between Linux and macOS we need a
# higher rms threshold for this test to pass
# GRAPHICSMAGICK_RMS = 0.0565
ps=gpsgridder2.ps
#V=-Vl
INC=5m
DEC=2
gmt select @wus_gps_final.txt -R122.5W/115W/32.5N/40N -fg -o0:5 > data.lluv
# Use blockmean to avoid aliasing
R=-R122.5W/115W/32.5N/38N
gmt blockmean $R -I${INC} data.lluv -fg -i0:2,4 -W > blk.llu
gmt blockmean $R -I${INC} data.lluv -fg -i0,1,3,5 -W > blk.llv
gmt convert -A blk.llu blk.llv -o0:2,6,3,7 > blk.lluv
#
#  do the gridding. There are 2682 data so use about 1/4 this number of singular values
#
gmt gpsgridder $R -I${INC} -Gtmp_%s.nc blk.lluv -fg $V -Fd8 -Cn0.25+feigen.txt -S0.5 -W
#
#  mask the grids
#
gmt grdlandmask -Gtmp_mask1.grd -Rtmp_u.nc -Df
gmt grdmask corner.ll -Gtmp_mask2.grd -Rtmp_u.nc -N1/0/0
gmt grdmath tmp_mask1.grd tmp_mask2.grd MUL 0 NAN = mask.grd
gmt grdmath tmp_u.nc mask.grd MUL = GPS_u.grd
gmt grdmath tmp_v.nc mask.grd MUL = GPS_v.grd

# make a plot of GPS velocity vectors
#
#
gmt set FORMAT_GEO_MAP dddF
gmt select blk.lluv $R -fg | awk '{ print($0," 0 ") }' > data.lluvenct
#
#   first make a mask
#
gmt grdlandmask -Gtmp_mask1.grd -RGPS_u.grd -Df
gmt grdmask corner.ll -Gtmp_mask2.grd -RGPS_u.grd -N1/0/0
gmt grdmath tmp_mask1.grd tmp_mask2.grd MUL 0 NAN = mask.grd
gmt grdmath GPS_u.grd mask.grd MUL = GPS_u.nc
gmt grdmath GPS_v.grd mask.grd MUL = GPS_v.nc
#
#   make the map
#
gmt pscoast $R -JM7i -P -Glightgray -Ba1f30m -BWSne -K -Df -X1i -Wfaint > $ps
gmt psxy @CA_fault_data.txt -J -R -W0.5p -O -K >> $ps
gmt psvelo data.lluvenct -J -R -Se.008i/0.95+f8p -A9p -W0.2p,red -O -K >> $ps
# Shrink down heads of vectors shorter than 10 km
gmt grdvector GPS_u.nc GPS_v.nc -Ix${DEC}/${DEC} -J -R -O -K -Q0.06i+e+n10 -Gblue -W0.2p,blue -S100i --MAP_VECTOR_SHAPE=0.2 >> $ps
#
# Place the scale using a geovector of length RATE
#
RATE=50 # This is in mm/yr or km/Myr - change to use another scalebar
echo 121.5W 33N 90 ${RATE}k   | gmt psxy   -R -J -O -K -S=0.06i+e+jc -Gblue -W1p,blue --MAP_VECTOR_SHAPE=0.2 >> $ps
echo 121.5W 33N ${RATE} mm/yr | gmt pstext -R -J -O -F+f8p+jCB -D0/0.07i >> $ps
