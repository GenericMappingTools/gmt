#!/bin/sh

set -e

# Get the target OS and CPU
CPU=`uname -p`
OS=`uname`


#Constants
FILE1=tst_diskless.nc
FILE2=tst_diskless2.nc
FILE3=tst_diskless3.nc

echo ""
echo "*** Testing in-memory (diskless) files with mmap"

HASNC4=`../nc-config --has-nc4`

echo "**** Test diskless+mmap netCDF classic file without persistence"
./tst_diskless mmap
echo "PASS: diskless+mmap netCDF classic file without persistence"

echo ""
echo "**** Test diskless+mmap netCDF classic file with persistence"
rm -f $FILE1
./tst_diskless mmap persist
if test -f $FILE1 ; then
echo "**** $FILE1 created"
# ../ncdump/ncdump $FILE1
echo "PASS: diskless+mmap netCDF classic file with persistence"
else
echo "#### $FILE1 not created"
echo "FAIL: diskless+mmap netCDF classic file with persistence"
fi

rm -f tmp1.cdl tmp2.cdl tmp1.nc tmp2.nc

echo ""
echo "**** Testing nc_open in-memory (diskless+mmap) files"

# clear old files
rm -f tst_diskless3_file.cdl tst_diskless3_memory.cdl

echo ""
echo "**** Create and modify file without using diskless+mmap"
rm -f $FILE3
./tst_diskless3
../ncdump/ncdump $FILE3 >tst_diskless3_file.cdl

echo ""
echo "**** Create and modify file using diskless+mmap"
rm -f $FILE3
./tst_diskless3 diskless mmap
../ncdump/ncdump $FILE3 >tst_diskless3_memory.cdl

# compare
diff tst_diskless3_file.cdl tst_diskless3_memory.cdl

# cleanup
rm -f $FILE3 tst_diskless3_file.cdl tst_diskless3_memory.cdl

exit
