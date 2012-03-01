#!/bin/sh
# For a netCDF-4 build, test nccopy on netCDF files in this directory

set -e
echo ""

# These files are actually in $srcdir in distcheck builds, so they
# need to be handled differently.
# ref_tst_compounds2 ref_tst_compounds3 ref_tst_compounds4 
TESTFILES='tst_comp tst_comp2 tst_enum_data tst_fillbug 
 tst_group_data tst_nans tst_opaque_data tst_solar_1 tst_solar_2
 tst_solar_cmp tst_special_atts tst_string_data tst_unicode
 tst_vlen_data'

echo "*** Testing netCDF-4 features of nccopy on ncdump/*.nc files"
for i in $TESTFILES ; do
    echo "*** copy $i.nc to copy_of_$i.nc ..."
    ./nccopy $i.nc copy_of_$i.nc
    ./ncdump -n copy_of_$i $i.nc > tmp.cdl
    ./ncdump copy_of_$i.nc > copy_of_$i.cdl
    echo "*** compare " with copy_of_$i.cdl
    diff copy_of_$i.cdl tmp.cdl
    rm copy_of_$i.nc copy_of_$i.cdl tmp.cdl
done
echo "*** Create deflatable file for testing ..."
./tst_compress
echo "*** Test nccopy -d1 can compress a file ..."
./nccopy -d1 tst_inflated.nc tst_deflated.nc
if test `wc -c < tst_deflated.nc` -ge  `wc -c < tst_inflated.nc`; then
    exit 1
fi
echo "*** Test nccopy -d1 -s can compress even more ..."
./nccopy -d1 -s tst_inflated.nc tmp.nc
if test `wc -c < tmp.nc` -ge  `wc -c < tst_inflated.nc`; then
    exit 1
fi
rm tst_deflated.nc tst_inflated.nc tmp.nc 

echo "*** Testing nccopy -d1 -s on ncdump/*.nc files"
for i in $TESTFILES ; do
    echo "*** nccopy -d1 -s $i.nc copy_of_$i.nc ..."
    ./nccopy -d1 -s $i.nc copy_of_$i.nc
    ./ncdump -n copy_of_$i $i.nc > tmp.cdl
    ./ncdump copy_of_$i.nc > copy_of_$i.cdl
    echo "*** compare " with copy_of_$i.cdl
    diff copy_of_$i.cdl tmp.cdl
    rm copy_of_$i.nc copy_of_$i.cdl tmp.cdl
done
echo "*** Create chunkable file for testing ..."
./tst_chunking
echo "*** Test that nccopy -c can chunk and unchunk files"
./nccopy tst_chunking.nc tmp.nc
./ncdump tmp.nc > tmp.cdl
./nccopy -c dim0/,dim1/1,dim2/,dim3/1,dim4/,dim5/1,dim6/ tst_chunking.nc tmp-chunked.nc
./ncdump -n tmp tmp-chunked.nc > tmp-chunked.cdl
diff tmp.cdl tmp-chunked.cdl
./nccopy -c dim0/,dim1/,dim2/,dim3/,dim4/,dim5/,dim6/ tmp-chunked.nc tmp-unchunked.nc
./ncdump -n tmp tmp-unchunked.nc > tmp-unchunked.cdl
diff tmp.cdl tmp-unchunked.cdl
# echo "*** Test that nccopy compression with chunking can improve compression"
rm tst_chunking.nc tmp.nc tmp.cdl tmp-chunked.nc tmp-chunked.cdl tmp-unchunked.nc tmp-unchunked.cdl

echo
echo "*** All nccopy tests passed!"
exit 0
