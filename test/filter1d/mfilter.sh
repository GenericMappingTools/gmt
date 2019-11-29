#!/usr/bin/env bash
# Get and filter the Keeling curve with monthly averages
ps=mfilter.ps
gmt set MAP_ANNOT_ORTHO "" FORMAT_TIME_PRIMARY_MAP Abbreviated FORMAT_DATE_MAP o
URL=https://scrippsco2.ucsd.edu/assets/data/atmospheric/stations/in_situ_co2/weekly/weekly_in_situ_co2_mlo.csv
gmt filter1d $URL -T1o+t -Fg30 --TIME_UNIT=d -f0T --IO_HEADER_MARKER=% > monthly.txt
gmt psxy -R1958T/2018T/300/430 -JX6iT/4i -Bxa10Yf5y -Byaf+u" ppm" -BWSne -W0.25p monthly.txt -P -K -Xc > $ps
gmt psxy -R2012T/2013T/380/410 -J -Bsxa1Y -Bpxa1Og1o -Byaf+u" ppm" -BWSne+t"Keeling CO@-2@- curve" $URL -Sc0.2c -Gred -O -K -Y4.75i --IO_HEADER_MARKER=% >> $ps
gmt psxy -R -J -Sx0.2i -W1p,blue monthly.txt -O >> $ps
