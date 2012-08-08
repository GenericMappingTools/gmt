#!/bin/sh
# This shell script tests ncdump for netcdf-4
# $Id$

set -e
echo ""
echo "*** Testing ncgen and ncdump test output for netCDF-4 format."
echo "*** creating netcdf-4 file c0.nc from c0.cdl..."
../ncgen/ncgen -k3 -b -o c0.nc $srcdir/../ncgen/c0.cdl
echo "*** creating c1.cdl from c0.nc..."
./ncdump -n c1 c0.nc > c1.cdl
echo "*** comparing c1.cdl with ref_ctest1_nc4.cdl..."
diff c1.cdl $srcdir/ref_ctest1_nc4.cdl

echo
echo "*** Testing ncgen and ncdump test output for netCDF-4 classic format."
echo "*** creating netcdf-4 classic file c0.nc from c0.cdl..."
../ncgen/ncgen -k4 -b -o c0.nc $srcdir/../ncgen/c0.cdl
echo "*** creating c1.cdl from c0.nc..."
./ncdump -n c1 c0.nc > c1.cdl
echo "*** comparing c1.cdl with ref_ctest1_nc4c.cdl..."
diff c1.cdl $srcdir/ref_ctest1_nc4c.cdl

echo
echo "*** Testing ncdump output for netCDF-4 features."
echo "*** dumping tst_solar_1.nc to tst_solar_1.cdl..."
./ncdump tst_solar_1.nc > tst_solar_1.cdl
echo "*** comparing tst_solar_1.cdl with ref_tst_solar_1.cdl..."
diff tst_solar_1.cdl $srcdir/ref_tst_solar_1.cdl
echo "*** dumping tst_solar_2.nc to tst_solar_2.cdl..."
./ncdump tst_solar_2.nc > tst_solar_2.cdl
echo "*** comparing tst_solar_2.cdl with ref_tst_solar_2.cdl..."
diff tst_solar_2.cdl $srcdir/ref_tst_solar_2.cdl
echo "*** dumping tst_group_data.nc to tst_group_data.cdl..."
./ncdump tst_group_data.nc > tst_group_data.cdl
echo "*** comparing tst_group_data.cdl with ref_tst_group_data.cdl..."
diff tst_group_data.cdl $srcdir/ref_tst_group_data.cdl
echo "*** testing -v option with absolute name and groups..."
./ncdump -v /g2/g3/var tst_group_data.nc > tst_group_data.cdl
echo "*** comparing tst_group_data.cdl with ref_tst_group_data_v23.cdl..."
diff tst_group_data.cdl $srcdir/ref_tst_group_data_v23.cdl
echo "*** testing -v option with relative name and groups..."
./ncdump -v var,var2 tst_group_data.nc > tst_group_data.cdl
echo "*** comparing tst_group_data.cdl with ref_tst_group_data.cdl..."
diff tst_group_data.cdl $srcdir/ref_tst_group_data.cdl
echo "*** dumping tst_enum_data.nc to tst_enum_data.cdl..."
./ncdump tst_enum_data.nc > tst_enum_data.cdl
echo "*** comparing tst_enum_data.cdl with ref_tst_enum_data.cdl..."
diff tst_enum_data.cdl $srcdir/ref_tst_enum_data.cdl
echo "*** dumping tst_opaque_data.nc to tst_opaque_data.cdl..."
./ncdump tst_opaque_data.nc > tst_opaque_data.cdl
echo "*** comparing tst_opaque_data.cdl with ref_tst_opaque_data.cdl..."
diff tst_opaque_data.cdl $srcdir/ref_tst_opaque_data.cdl
echo "*** dumping tst_vlen_data.nc to tst_vlen_data.cdl..."
./ncdump tst_vlen_data.nc > tst_vlen_data.cdl
echo "*** comparing tst_vlen_data.cdl with ref_tst_vlen_data.cdl..."
diff tst_vlen_data.cdl $srcdir/ref_tst_vlen_data.cdl
echo "*** dumping tst_comp.nc to tst_comp.cdl..."
./ncdump tst_comp.nc > tst_comp.cdl
echo "*** comparing tst_comp.cdl with ref_tst_comp.cdl..."
diff tst_comp.cdl $srcdir/ref_tst_comp.cdl
echo "*** creating tst_nans.cdl from tst_nans.nc"
./ncdump tst_nans.nc > tst_nans.cdl
echo "*** comparing ncdump of generated file with ref_tst_nans.cdl ..."
diff tst_nans.cdl $srcdir/ref_tst_nans.cdl
# Do unicode test only if it exists => BUILD_UTF8 is true
if test -f ./tst_unicode -o -f ./tst_unicode.exe ; then
  echo "*** dumping tst_unicode.nc to tst_unicode.cdl..."
  ./tst_unicode
  ./ncdump tst_unicode.nc > tst_unicode.cdl
  #echo "*** comparing tst_unicode.cdl with ref_tst_unicode.cdl..."
  #diff tst_unicode.cdl $srcdir/ref_tst_unicode.cdl
  echo "*** generating tst_nans.nc"
fi
./tst_special_atts
echo "*** dumping tst_special_atts.nc to tst_special_atts.cdl..."
./ncdump -c -s tst_special_atts.nc > tst_special_atts.cdl
echo "*** comparing tst_special_atts.cdl with ref_tst_special_atts.cdl..."
diff tst_special_atts.cdl $srcdir/ref_tst_special_atts.cdl

echo
echo "*** All ncgen and ncdump test output for netCDF-4 format passed!"
exit 0
