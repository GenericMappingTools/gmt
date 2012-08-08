#!/bin/sh
set -x

# This shell gets files from the netCDF ftp site for testing.

set -e
echo ""
file_list="MSGCPP_CWP_NC3.nc  MSGCPP_CWP_NC4.nc"
echo "Getting KNMI test files $file_list"

for f1 in $file_list
do
    if ! test -f $f1; then
	wget ftp://ftp.unidata.ucar.edu/pub/netcdf/sample_data/$f1.gz
	gunzip $f1.gz
    fi
done

echo "SUCCESS!!!"

exit 0
