#!/bin/sh
# This shell script runs the backward compatibility tests.

set -e
echo ""
echo "*** Testing that this version can read data produced by old versions of netCDF."
echo "*** checking ref_nc_test_netcdf4_4_0.nc..."
./ncdump $srcdir/ref_nc_test_netcdf4_4_0.nc > tst_nc_test_netcdf4_4_0.cdl
tail -n +2 < $srcdir/ref_nc_test_netcdf4.cdl > tmp.cdl
tail -n +2 < tst_nc_test_netcdf4_4_0.cdl > tmp_4_0.cdl
diff -w tmp.cdl tmp_4_0.cdl

# echo "*** Testing that old versions can read data produced by this version of netCDF."
# echo "*** checking version 4.0..."
# ../ncgen/ncgen -b -o tst_nc_test_netcdf4 -k 4 $srcdir/ref_nc_test_netcdf4.cdl
# /machine/local_4.0/bin/ncdump tst_nc_test_netcdf4.nc > tst_nc_test_netcdf4.cdl
# tail -n +2 <$srcdir/ref_nc_test_netcdf4.cdl > tmp.cdl
# tail -n +2 <tst_nc_test_netcdf4.cdl > tmp_4_0.cdl
# diff -w tmp.cdl tmp_4_0.cdl

# echo "*** checking version 4.1.1..."
# ../ncgen/ncgen -b -o tst_nc_test_netcdf4 -k 4 $srcdir/ref_nc_test_netcdf4.cdl
# /machine/local_4.1.1/bin/ncdump tst_nc_test_netcdf4.nc > tst_nc_test_netcdf4.cdl
# tail -n +2 <$srcdir/ref_nc_test_netcdf4.cdl > tmp.cdl
# tail -n +2 <tst_nc_test_netcdf4.cdl > tmp_4_0.cdl
# diff -w tmp.cdl tmp_4_0.cdl

echo "*** All backward compatibility tests passed!"
exit 0
