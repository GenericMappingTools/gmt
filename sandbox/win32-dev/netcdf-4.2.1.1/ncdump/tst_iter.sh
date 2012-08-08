#!/bin/sh
# This shell script runs an ncdump bug test for netcdf
# Test if the nciter code is working [NCF-154]

set -e
echo ""
echo "*** Running ncdump nc_iter test."

if test "x$CC" = "x" ; then CC="gcc"; fi

CLEANUP="iter.*"

rm -f $CLEANUP

# echo "create iter.cdl"
cat > iter.cdl <<EOF
netcdf iter {
dimensions:
x = 5 ;
y = 256 ;
z = 256;
variables:
int var(x,y,z) ;
data:
var =
EOF
cat >./iter.c <<EOF
#include <stdlib.h>
#include <stdio.h>
#define N (5*256*256)
int main() {
  int i;
  for(i=0;i<N-1;i++) {printf("%d,\n",i);}
  printf("%d;\n}\n",N);
  return 0;
}
EOF

$CC ./iter.c -o iter.exe
./iter.exe >>iter.cdl

# echo "*** create iter.nc "
../ncgen/ncgen -k1 -o iter.nc ./iter.cdl
echo "*** dumping iter.nc to iter.dmp"
./ncdump iter.nc > iter.dmp
echo "*** reformat iter.dmp"
mv iter.dmp iter.tmp
sed -e 's/\([0-9][,]\) /\1@/g' <iter.tmp |tr '@' '\n' |sed -e '/^$/d' >./iter.dmp

echo "*** comparing iter.dmp with iter.cdl..."
diff -b -w ./iter.dmp ./iter.cdl

# cleanup
rm -f $CLEANUP

exit 0
