#!/usr/bin/env bash
#
#	Testing gmt mapproject pt.txt with proj4 cases.
#
#	Slop is 1 cm for projected values
rm -f results.txt
echo 4.897 52.371 > pt.txt
gmt mapproject pt.txt -J+proj=aea+ellps=WGS84+units=m+lat_1=55+lat_2=65 | gmt math -o1 STDIN -C0 334609.583974 SUB -C1 5218502.503686 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=cass+ellps=WGS84+units=m | gmt math -o1 STDIN -C0 333274.431072 SUB -C1 5815921.803069 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt "-J+proj=cea+ellps=WGS84+units=m+lon_0=11d32'00E" | gmt math -o1 STDIN -C0 -738753.247401 SUB -C1 5031644.669407 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=eqdc+ellps=WGS84+units=m+lat_1=60+lat_2=0 | gmt math -o1 STDIN -C0 307874.536263 SUB -C1 5810915.646438 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=hammer+ellps=WGS84+units=m | gmt math -o1 STDIN -C0 370843.923425 SUB -C1 5630047.232233 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=lcc+ellps=WGS84+units=m+lat_1=20n+lat_2=60n | gmt math -o1 STDIN -C0 319700.820572 SUB -C1 5828852.504871 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=merc+ellps=WGS84+units=m | gmt math -o1 STDIN -C0 545131.546415 SUB -C1 6833623.829215 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=mill+ellps=WGS84+units=m+wktext | gmt math -o1 STDIN -C0 545131.546415 SUB -C1 6431916.372717 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=moll+ellps=WGS84+units=m | gmt math -o1 STDIN -C0 360567.451176 SUB -C1 6119459.421291 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=poly+ellps=WGS84+units=m | gmt math -o1 STDIN -C0 333274.269648 SUB -C1 5815908.957562 SUB 0 COL HYPOT 0.01 GT = >> results.txt
# gmt mapproject pt.txt -J+proj=robin+ellps=WGS84+units=m | gmt math -o1 STDIN -C0 394576.507489 SUB -C1 5571243.439235 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=sinu+ellps=WGS84+units=m | gmt math -o1 STDIN -C0 333528.909809 SUB -C1 5804625.044313 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=tmerc+ellps=WGS84+units=m | gmt math -o1 STDIN -C0 333425.492136 SUB -C1 5815921.814396 SUB 0 COL HYPOT 0.01 GT = >> results.txt
#gmt mapproject pt.txt -J+proj=utm+ellps=WGS84+units=m+lon_0=11d32'00E | gmt math -o1 STDIN -C0 220721.998929 SUB -C1 5810228.672907 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=vandg+ellps=WGS84+units=m+wktext | gmt math -o1 STDIN -C0 489005.929978 SUB -C1 6431581.024949 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=omerc+ellps=WGS84+units=m+lat_1=20n+lat_2=60n+lon_1=1e+lon_2=30e+wktext | gmt math -o1 STDIN -C0 1009705.329154 SUB -C1 5829437.254923 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=oea+ellps=WGS84+units=m+lat_1=20n+lat_2=60n+lon_1=1e+lon_2=30e+m=1+n=1 | gmt math -o1 STDIN -C0 545723.850088 SUB -C1 5058869.127694 SUB 0 COL HYPOT 0.01 GT = >> results.txt
# The airy test is failing for GDAL3 for its own (GDAL) reasons.
#gmt mapproject pt.txt -J+proj=airy+ellps=WGS84+units=m | gmt math -o1 STDIN -C0 328249.003313 SUB -C1 4987937.101447 SUB 0 COL HYPOT 0.01 GT = >> results.txt
# gmt mapproject pt.txt -J+proj=aeqd+ellps=WGS84+units=m | gmt math -o1 STDIN -C0 384923.723428 SUB -C1 5809986.497118 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=laea+ellps=WGS84+units=m | gmt math -o1 STDIN -C0 371541.476735 SUB -C1 5608007.251007 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=stere+ellps=WGS84+units=m+lat_ts=30n | gmt math -o1 STDIN -C0 414459.6218269 SUB -C1 6255826.7498713 SUB 0 COL HYPOT 0.01 GT = >> results.txt
gmt mapproject pt.txt -J+proj=sterea+lat_0=52.15616055555555+lon_0=5.38763888888889+k=0.9999079+x_0=155000+y_0=463000+ellps=bessel+units=m | gmt math -o1 STDIN -C0 121590.388077 SUB -C1 487013.903377 SUB 0 COL HYPOT 0.01 GT = >> results.txt
errors=`gmt math -T -Sl results.txt SUM =`
if [ $errors -ne 0 ]; then
	cp -f results.txt fail
fi
