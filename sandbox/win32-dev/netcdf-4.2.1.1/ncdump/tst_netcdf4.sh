#!/bin/sh
# This shell script tests ncdump for netcdf-4
# $Id$

set -e

if test "x$srcdir" = "x"; then
    srcdir=`dirname $0`; 
fi
# add hack for sunos
export srcdir;

echo ""
echo "*** Testing ncgen and ncdump test output for netCDF-4 format."
# echo "*** creating netcdf-4 file c0.nc from c0.cdl..."
../ncgen/ncgen -k3 -b -o c0.nc $srcdir/../ncgen/c0.cdl
# echo "*** creating c1.cdl from c0.nc..."
./ncdump -n c1 c0.nc | sed 's/e+0/e+/g' > c1.cdl
# echo "*** comparing c1.cdl with ref_ctest1_nc4.cdl..."
diff -b c1.cdl $srcdir/ref_ctest1_nc4.cdl

echo "*** Testing ncgen and ncdump test output for netCDF-4 classic format."
# echo "*** creating netcdf-4 classic file c0.nc from c0.cdl..."
../ncgen/ncgen -k4 -b -o c0.nc $srcdir/../ncgen/c0.cdl
# echo "*** creating c1.cdl from c0.nc..."
./ncdump -n c1 c0.nc | sed 's/e+0/e+/g' > c1.cdl
# echo "*** comparing c1.cdl with ref_ctest1_nc4c.cdl..."
diff -b c1.cdl $srcdir/ref_ctest1_nc4c.cdl

echo "*** Testing ncdump output for netCDF-4 features."
./ncdump tst_solar_1.nc | sed 's/e+0/e+/g' > tst_solar_1.cdl
# echo "*** comparing tst_solar_1.cdl with ref_tst_solar_1.cdl..."
diff -b tst_solar_1.cdl $srcdir/ref_tst_solar_1.cdl
./ncdump tst_solar_2.nc | sed 's/e+0/e+/g' > tst_solar_2.cdl
# echo "*** comparing tst_solar_2.cdl with ref_tst_solar_2.cdl..."
diff -b tst_solar_2.cdl $srcdir/ref_tst_solar_2.cdl
./ncdump tst_group_data.nc | sed 's/e+0/e+/g' > tst_group_data.cdl
# echo "*** comparing tst_group_data.cdl with ref_tst_group_data.cdl..."
diff -b tst_group_data.cdl $srcdir/ref_tst_group_data.cdl

# Temporary hack to skip a couple tests that won't work in windows
# without changing the format of the string. See:
#
# http://www.mingw.org/wiki/Posix_path_conversion

if [[ "$OSTYPE" != 'msys' ]]; then
echo "*** Testing -v option with absolute name and groups..."
./ncdump -v /g2/g3/var tst_group_data.nc | sed 's/e+0/e+/g' > tst_group_data.cdl
# echo "*** comparing tst_group_data.cdl with ref_tst_group_data_v23.cdl..."
diff -b tst_group_data.cdl $srcdir/ref_tst_group_data_v23.cdl
fi


echo "*** Testing -v option with relative name and groups..."
./ncdump -v var,var2 tst_group_data.nc | sed 's/e+0/e+/g' > tst_group_data.cdl
# echo "*** comparing tst_group_data.cdl with ref_tst_group_data.cdl..."
diff -b tst_group_data.cdl $srcdir/ref_tst_group_data.cdl
./ncdump tst_enum_data.nc | sed 's/e+0/e+/g' > tst_enum_data.cdl
# echo "*** comparing tst_enum_data.cdl with ref_tst_enum_data.cdl..."
diff -b tst_enum_data.cdl $srcdir/ref_tst_enum_data.cdl
./ncdump tst_opaque_data.nc | sed 's/e+0/e+/g' > tst_opaque_data.cdl
# echo "*** comparing tst_opaque_data.cdl with ref_tst_opaque_data.cdl..."
diff -b tst_opaque_data.cdl $srcdir/ref_tst_opaque_data.cdl
./ncdump tst_vlen_data.nc | sed 's/e+0/e+/g' > tst_vlen_data.cdl
# echo "*** comparing tst_vlen_data.cdl with ref_tst_vlen_data.cdl..."
diff -b tst_vlen_data.cdl $srcdir/ref_tst_vlen_data.cdl
./ncdump tst_comp.nc | sed 's/e+0/e+/g' > tst_comp.cdl
# echo "*** comparing tst_comp.cdl with ref_tst_comp.cdl..."
diff -b tst_comp.cdl $srcdir/ref_tst_comp.cdl
# echo "*** creating tst_nans.cdl from tst_nans.nc"
./ncdump tst_nans.nc | sed 's/e+0/e+/g' > tst_nans.cdl
# echo "*** comparing ncdump of generated file with ref_tst_nans.cdl ..."
diff -b tst_nans.cdl $srcdir/ref_tst_nans.cdl
# Do unicode test only if it exists => BUILD_UTF8 is true
if test -f ./tst_unicode -o -f ./tst_unicode.exe ; then
  echo "*** dumping tst_unicode.nc to tst_unicode.cdl..."
  ./tst_unicode
  ./ncdump tst_unicode.nc | sed 's/e+0/e+/g' > tst_unicode.cdl
  #echo "*** comparing tst_unicode.cdl with ref_tst_unicode.cdl..."
  #diff -b tst_unicode.cdl $srcdir/ref_tst_unicode.cdl
fi
./tst_special_atts
./ncdump -c -s tst_special_atts.nc | sed 's/e+0/e+/g' > tst_special_atts.cdl
echo "*** comparing tst_special_atts.cdl with ref_tst_special_atts.cdl..."
diff -b tst_special_atts.cdl $srcdir/ref_tst_special_atts.cdl

echo "*** All ncgen and ncdump test output for netCDF-4 format passed!"
exit 0
