#!/bin/sh
# This shell script runs the cmp test on the example programs.
# $Id$

set -e
echo ""
echo "*** Testing that CXX4 examples produced netCDF-4 files equivalent to C examples."
echo "*** checking simple_xy.nc..."
# cmp simple_xy.nc ../C/simple_xy.nc
../../ncdump/ncdump simple_xy.nc > tmp-test.cdl
../../ncdump/ncdump ../C/simple_xy.nc > tmp-ref.cdl
diff tmp-test.cdl tmp-ref.cdl

echo "*** checking sfc_pres_temp.nc..."
# cmp sfc_pres_temp.nc ../C/sfc_pres_temp.nc
../../ncdump/ncdump sfc_pres_temp.nc > tmp-test.cdl
../../ncdump/ncdump ../C/sfc_pres_temp.nc > tmp-ref.cdl
diff tmp-test.cdl tmp-ref.cdl

echo "*** checking pres_temp_4D.nc..."
# cmp pres_temp_4D.nc ../C/pres_temp_4D.nc
../../ncdump/ncdump pres_temp_4D.nc > tmp-test.cdl
../../ncdump/ncdump ../C/pres_temp_4D.nc > tmp-ref.cdl
diff tmp-test.cdl tmp-ref.cdl

echo "*** checking simple_xy_nc4.nc..."
# cmp simple_xy.nc ../C/simple_xy.nc
../../ncdump/ncdump -n simple_xy simple_xy_nc4.nc > tmp-test.cdl
../../ncdump/ncdump ../C/simple_xy.nc > tmp-ref.cdl
diff tmp-test.cdl tmp-ref.cdl

# The following three tests don't work yet.  Uncomment when
# simple_xy_wr_formats works to write files of nc4classic, classic,
# and classic64 formats.

# echo "*** checking simple_xy_nc4classic.nc..."
# # cmp simple_xy.nc ../C/simple_xy.nc
# ../../ncdump/ncdump -n simple_xy simple_xy_nc4classic.nc > tmp-test.cdl
# ../../ncdump/ncdump ../C/simple_xy.nc > tmp-ref.cdl
# diff tmp-test.cdl tmp-ref.cdl

# echo "*** checking simple_xy_classic.nc..."
# # cmp simple_xy.nc ../C/simple_xy.nc
# ../../ncdump/ncdump -n simple_xy simple_xy_classic.nc > tmp-test.cdl
# ../../ncdump/ncdump ../C/simple_xy.nc > tmp-ref.cdl
# diff tmp-test.cdl tmp-ref.cdl

# echo "*** checking simple_xy_classic64.nc..."
# # cmp simple_xy.nc ../C/simple_xy.nc
# ../../ncdump/ncdump -n simple_xy simple_xy_classic64.nc > tmp-test.cdl
# ../../ncdump/ncdump ../C/simple_xy.nc > tmp-ref.cdl
# diff tmp-test.cdl tmp-ref.cdl

echo "*** All CXX example comparisons worked!"
exit 0
