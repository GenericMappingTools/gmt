#!/bin/sh
# This shell script runs extra tests ncdump for netcdf-4
# $Id$

set -e
if test "x$srcdir" = "x"; then
    srcdir=`dirname $0`; 
fi
export srcdir

echo ""
echo "*** Running extra netcdf-4 tests."

#
# In windows, these tests fail because srcdir is prepended.
# e.g., Instead of 'ncdump ref_tst_compounds2' the file would
# contain:
#        'ncdump ./ref_tst_compounds2'. This causes the test to fail.
# But, 'srcdir' is necessary for make distcheck.
#
# Short term solution, use sed when on windows/MSYS to 
# remove the './','../../ncdump'.
# 

if [ `uname | cut -d "_" -f 1` = "MINGW32" ]; then # MINGW Platforms

    echo "*** dumping tst_string_data.nc to tst_string_data.cdl..."

    ./ncdump tst_string_data.nc > tst_string_data.cdl
    TMPNAME=`head -n 1 tst_string_data.cdl | cut -d" " -f 2`
    NEWNAME=`basename $TMPNAME`
    sed "s,$TMPNAME,$NEWNAME,g" tst_string_data.cdl > tmp.cdl
    mv tmp.cdl tst_string_data.cdl
    echo "*** comparing tst_string_data.cdl with ref_tst_string_data.cdl..."
    diff -b tst_string_data.cdl $srcdir/ref_tst_string_data.cdl

    echo '*** testing reference file ref_tst_compounds2.nc...'
    ./ncdump $srcdir/ref_tst_compounds2.nc > tst_compounds2.cdl
    TMPNAME=`head -n 1 tst_compounds2.cdl | cut -d" " -f 2`
    NEWNAME=`basename $TMPNAME`
    sed "s,$TMPNAME,$NEWNAME,g" tst_compounds2.cdl > tmp.cdl
    mv tmp.cdl tst_compounds2.cdl
    diff -b tst_compounds2.cdl $srcdir/ref_tst_compounds2.cdl

    echo '*** testing reference file ref_tst_compounds3.nc...'
    ./ncdump $srcdir/ref_tst_compounds3.nc > tst_compounds3.cdl
    TMPNAME=`head -n 1 tst_compounds3.cdl | cut -d" " -f 2`
    NEWNAME=`basename $TMPNAME`
    sed "s,$TMPNAME,$NEWNAME,g" tst_compounds3.cdl > tmp.cdl
    mv tmp.cdl tst_compounds3.cdl
    diff -b tst_compounds3.cdl $srcdir/ref_tst_compounds3.cdl

    echo '*** testing reference file ref_tst_compounds4.nc...'
    ./ncdump $srcdir/ref_tst_compounds4.nc > tst_compounds4.cdl
    TMPNAME=`head -n 1 tst_compounds4.cdl | cut -d" " -f 2`
    NEWNAME=`basename $TMPNAME`
    sed "s,$TMPNAME,$NEWNAME,g" tst_compounds4.cdl > tmp.cdl
    mv tmp.cdl tst_compounds4.cdl
    diff -b tst_compounds4.cdl $srcdir/ref_tst_compounds4.cdl

else # Non-MINGW Platforms

    echo "*** dumping tst_string_data.nc to tst_string_data.cdl..."
    ./ncdump tst_string_data.nc > tst_string_data.cdl
    echo "*** comparing tst_string_data.cdl with ref_tst_string_data.cdl..."
    diff -b tst_string_data.cdl $srcdir/ref_tst_string_data.cdl

#echo '*** testing non-coordinate variable of same name as dimension...'
#../ncgen/ncgen -v4 -b -o tst_noncoord.nc $srcdir/ref_tst_noncoord.cdl

    echo '*** testing reference file ref_tst_compounds2.nc...'
    ./ncdump $srcdir/ref_tst_compounds2.nc > tst_compounds2.cdl
    diff -b tst_compounds2.cdl $srcdir/ref_tst_compounds2.cdl

    echo '*** testing reference file ref_tst_compounds3.nc...'
    ./ncdump $srcdir/ref_tst_compounds3.nc > tst_compounds3.cdl
    diff -b tst_compounds3.cdl $srcdir/ref_tst_compounds3.cdl

    echo '*** testing reference file ref_tst_compounds4.nc...'
    ./ncdump $srcdir/ref_tst_compounds4.nc > tst_compounds4.cdl
    diff -b tst_compounds4.cdl $srcdir/ref_tst_compounds4.cdl

fi

echo "*** All ncgen and ncdump extra test output for netCDF-4 format passed!"
exit 0

