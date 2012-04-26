#!/bin/bash
#	$Id$
#
# Tests mgd77convert

log=convert.log

OLD=$MGD77_HOME
export MGD77_HOME=${GMT_SOURCE_DIR}/share/mgd77
ln -fs "${src:=.}"/dummy.mgd77 12345678.mgd77

# Make cdf file
mgd77convert 12345678 -Fa -Tc
# Make dat file
mgd77convert 12345678 -Fa -Tt

echo "Test if dat files are the same if coming from mgd77 or nc:" > $log
mv 12345678.dat tt.dat
mgd77convert 12345678 -Fc -Tt
diff 12345678.dat tt.dat --strip-trailing-cr | tee fail >> $log
echo "Test if mgd77 files are the same if coming from nc or dat:" >> $log
rm -f 12345678.mgd77
mgd77convert 12345678 -Ft -Ta
mv 12345678.mgd77 tt.mgd77
mgd77convert 12345678 -Fc -Ta
diff 12345678.mgd77 tt.mgd77 --strip-trailing-cr | tee -a fail >> $log
echo "Test if mgd77 from nc matches original:" >> $log
diff 12345678.mgd77 dummy.mgd77 --strip-trailing-cr | tee -a fail >> $log
echo "Test if mgd77 from dat matches original:" >> $log
diff tt.mgd77 dummy.mgd77 --strip-trailing-cr | tee -a fail >> $log

export MGD77_HOME=$OLD
