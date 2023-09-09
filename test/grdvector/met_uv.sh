#!/usr/bin/env bash
# testing vector head shrinking and non-plotting for plain and filled geovectors
# Using tiny subset of data shown in https://github.com/GenericMappingTools/gmt/issues/6108
# New behavior is skipping of heads when size exceeds length
gmt begin met_uv
	gmt set MAP_FRAME_TYPE plain FONT_TAG 12p
	gmt subplot begin 5x1 -Fs14c/4.21c -R84/96/24.75/28 -JM14c -Sct -Srl -M2p -A
		gmt subplot set -A"No heads"
		gmt grdvector uc2.nc vc2.nc -Ix5 -W0.5p -Si20k
		gmt subplot set -A"-Q14p+eA+n5k/0.5"
		gmt grdvector uc2.nc vc2.nc -Ix5 -Q14p+eA+n5k/0.5 -W1p -Si20k
		gmt subplot set -A"-Q14p+eA+n5k/0"
		gmt grdvector uc2.nc vc2.nc -Ix5 -Q14p+eA+n5k/0 -W1p -Si20k
		gmt subplot set -A"-Q14p+e+n5k/0.5"
		gmt grdvector uc2.nc vc2.nc -Ix5 -Q14p+e+n5k/0.5 -Gred -W1p -Si20k
		gmt subplot set -A"-Q14p+e+n5k/0"
		gmt grdvector uc2.nc vc2.nc -Ix5 -Q14p+e+n5k/0 -Gred -W1p -Si20k
	gmt subplot end
gmt end show
