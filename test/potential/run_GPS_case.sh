#!/bin/bash
#	$Id$
#	Testing gpsgridder for small vs large region, with our without weights
#
#  Get data, region, dinc, ginc, decimate, weight option from first six arguments
#
D=$1
R=-R$2
I=$3
ginc=$4
dec=$5
W=$6

#
# reformat the data and set the minimum sigma at 0.6 mm/yr
#
gmt select ${src:-.}/$D $R -fg | awk '{su = ($5 < .6 ? .6 : $5);sv = ($6 < .6 ? .6 : $6);print($1,$2,$3,$4,su,sv) }' > data.lluv

#
# Use blockmean to avoid aliasing
gmt blockmean $R -I$I data.lluv -fg -i0,1,2,4 -W > blk.llu
gmt blockmean $R -I$I data.lluv -fg -i0,1,3,5 -W > blk.llv
gmt convert -A blk.llu blk.llv -o0-2,6,3,7 > tmp.lluv
#
# change some of the e-component of the vectors
#
awk '{if($1 > -116.6 && $1 < -116.5 && $2 > 33.5 && $2 < 33.7) print($1,$2,-$3,$4,5*$5,5*$6); else print($0);}' < tmp.lluv > blk.lluv
#
#  do the gridding. Seem to work correctly wothout weights
#
gmt gpsgridder $R -I$ginc -GGPS_%s.nc blk.lluv -fg -Vl -Fd4 -r -Cn400+eigen.txt -S0.50 $W
#

gmt set FORMAT_GEO_MAP = dddF
gmt select blk.lluv $R -fg | awk '{ print($0," 0 ") }' > data.lluvenct
#
#   first make a mask
#
gmt grdlandmask -Gmask.nc -RGPS_u.nc -Df
gmt grdmath GPS_u.nc mask.nc MUL = GPS_u.nc
gmt grdmath GPS_v.nc mask.nc MUL = GPS_v.nc
#
#   make the map
#
gmt pscoast -R -JM7i -P -Glightgray -Ba1f30m -BWSne -K -Df -X0.7i -Wfaint
gmt psxy fault_data.gmt -J -R -W.5 -O -V -K
gmt psvelo data.lluvenct -J -R -Se.008i/0.95/8 -A9p -W.4,red -O -K
gmt grdvector GPS_u.nc GPS_v.nc -Ix$dec/$dec -J -R -O -Q0.03i+e+n0.03i -Gblue -W.4,blue -S1i --MAP_VECTOR_SHAPE=0.2
