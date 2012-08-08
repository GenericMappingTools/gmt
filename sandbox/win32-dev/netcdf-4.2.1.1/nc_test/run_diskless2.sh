#!/bin/sh

set -e

# Get the target OS and CPU
CPU=`uname -p`
OS=`uname`

#Constants
FILE4=tst_diskless4.nc

# Compute the file size for tst_diskless4
SIZE=0
case $CPU in
*_64*) SIZE=3000000000;;
*)     SIZE=1000000000;;
esac

# Create the reference ncdump output for tst_diskless4
rm -fr tst_diskless4.cdl
echo "netcdf tst_diskless4 {" >>tst_diskless4.cdl
echo "dimensions:" >>tst_diskless4.cdl
echo "	dim = 1000000000 ;" >>tst_diskless4.cdl
echo "variables:" >>tst_diskless4.cdl
echo "	byte var0(dim) ;" >>tst_diskless4.cdl
if test $SIZE = 3000000000 ; then
echo "	byte var1(dim) ;" >>tst_diskless4.cdl
echo "	byte var2(dim) ;" >>tst_diskless4.cdl
fi
echo "}" >>tst_diskless4.cdl

echo ""
rm -f $FILE4
time ./tst_diskless4 $SIZE create
# Validate it
../ncdump/ncdump -h $FILE4 |diff - tst_diskless4.cdl

echo ""
rm -f $FILE4
time ./tst_diskless4 $SIZE creatediskless
# Validate it
../ncdump/ncdump -h $FILE4 |diff - tst_diskless4.cdl

echo ""
time ./tst_diskless4 $SIZE open

echo ""
time ./tst_diskless4 $SIZE opendiskless

# cleanup
rm -f $FILE4 tst_diskless4.cdl

exit
