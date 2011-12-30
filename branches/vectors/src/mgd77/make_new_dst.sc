#! /bin/sh -f
#
# Creates a new Dst_.xxx coefficient file from "Est_Ist_index_0_mean.pli" that leaves at
# ftp://ftp.ngdc.noaa.gov/STP/GEOMAGNETIC_DATA/INDICES/EST_IST/
# Use the output of this script to update the Dst_all.wdc file
#
# Joaquim Luis

gmtconvert Est_Ist_index_0_mean.pli -fi0t --TIME_SYSTEM="days since 2000-01-01" -fo0T --OUTPUT_DATE_FORMAT=yyyy-mm-dd \
--OUTPUT_CLOCK_FORMAT=hh | awk -v som=0 '{if (NR % 24 == 0) printf("%4.3d%+04.0f\n", $2, (som+=$2)/24, som=0); \
else if (NR % 24 == 1) printf("DST%.2d%.2dP%.2d       000%4.3d", substr($1,3,2), substr($1,6,2), substr($1,9,2), $2, som+=$2); \
else printf("%4.3d", $2, som+=$2) }' > Dst_pli.dat
