#!/bin/sh

set -e
RESULTSDIR="./results"
#SHOWXFAILS=1

# Locate the cdl and expected directory
cdl="${srcdir}/cdl4"
expected="${srcdir}/expected4"

case "x${KFLAG}" in
x1) CLASSIC=1; MODE=3;;
x2) CLASSIC=1; MODE=3;;
x3) CLASSIC=0; MODE=4;;
x4) CLASSIC=1; MODE=3;;
*) echo "illegal KFLAG" ; exit 1;;
esac

# Define the set of tests that can be
# processed with either the -k1 or -k3 or -k4 flag

# The netcdf-3 tests are divided into two parts
# These test can be run when --enable-netcdf-4 is false
CLASSIC3="\
nc_enddef \
ref_tst_unicode \
ref_tst_utf8 \
simple_xy \
small \
nc_sync \
ref_tst_small \
small2 \
tst_ncml
n3time \
ref_tst_chardata \
unlimtest1"

NONCLASSIC3="\
test0 \
sfc_pres_temp \
fills \
c0 \
example_good \
pres_temp_4D \
ref_nctst \
ref_nctst_64bit_offset \
ref_ctest1_nc4 \
ref_ctest1_nc4c \
ref_nctst_netcdf4 \
ref_nctst_netcdf4_classic \
ref_tst_unlim2"

if test "${CLASSIC}" = "1" ; then
TESTS3="${CLASSIC3}"
else
TESTS3="${CLASSIC3} ${NONCLASSIC3}"
fi

# Define the set of tests that must be
# processed with the -k3 flag

TESTS4="\
ref_dimscope \
ref_typescope \
ref_tst_string_data \
ref_tst_comp \
ref_tst_comp2 \
ref_tst_comp3 \
ref_tst_group_data \
ref_tst_opaque_data \
ref_tst_vlen_data \
ref_tst_solar_1 \
ref_tst_solar_2 \
ref_tst_enum_data \
ref_tst_special_atts \
ref_tst_nans \
ref_solar \
unlimtest2"

SPECIALTESTS3="ref_tst_special_atts3"

SPECIALTESTS="ref_tst_special_atts ${SPECIALTESTS3}"

XFAILTESTS="ref_const_test ref_tst_unlim2 ref_tst_chardata"

# Following are generally not run
# Because of the size of their output
BIGTESTS3="\
bigf1 \
bigf2 \
bigr1 \
bigr2"

# Following deliberately produces a very large
# file: too large for netcdf to handle
# Currently not used because of space and time
# constraints
XFAILBIG="bigf3"

BIGTESTS4="ref_tst_solar_1"

# This test is both big and slow
# File was too large to reasonably include
# so I removed it
#BIGBIG3="gfs1"

BIGTESTS="${BIGTESTS3} ${BIGTESTS4} ${BIGBIG3}"

failcount=0
passcount=0
xfailcount=0

rm -fr results
mkdir results
