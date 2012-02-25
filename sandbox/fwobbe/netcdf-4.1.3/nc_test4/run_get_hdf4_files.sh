#!/bin/sh

# This shell gets files from the netCDF ftp site for testing.

# $Id$

set -e
echo ""
file_list="AMSR_E_L2_Rain_V10_200905312326_A.hdf AMSR_E_L3_DailyLand_V06_20020619.hdf \
    MYD29.A2009152.0000.005.2009153124331.hdf MYD29.A2002185.0000.005.2007160150627.hdf \
    MOD29.A2000055.0005.005.2006267200024.hdf"
echo "Getting HDF4 test files $file_list"

for f1 in $file_list
do
    if ! test -f $f1; then
	wget ftp://ftp.unidata.ucar.edu/pub/netcdf/sample_data/hdf4/$f1.gz
	gunzip $f1.gz
    fi
done

echo "SUCCESS!!!"

exit 0
