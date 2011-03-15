#!/bin/bash
#	$Id: convert.sh,v 1.6 2011-03-15 02:06:45 guru Exp $
#
# Tests mgd77convert

. ../functions.sh
header "Test mgd77convert conversions between mgd77-dat-nc"

log=convert.log

MGD77_HOME=${MGD77_HOME:-$GMTHOME/share/mgd77}
export MGD77_HOME
cp dummy.mgd77 12345678.mgd77

# Make cdf file
mgd77convert 12345678 -Fa -Tc
# Make dat file
mgd77convert 12345678 -Fa -Tt

echo "Test if dat files are the same if coming from mgd77 or nc:" > $log
mv 12345678.dat $$.dat
mgd77convert 12345678 -Fc -Tt
diff 12345678.dat $$.dat | tee fail >> $log
echo "Test if mgd77 files are the same if coming from nc or dat:" >> $log
rm -f 12345678.mgd77
mgd77convert 12345678 -Ft -Ta
mv 12345678.mgd77 $$.mgd77
mgd77convert 12345678 -Fc -Ta
diff 12345678.mgd77 $$.mgd77 | tee -a fail >> $log
echo "Test if mgd77 from nc matches original:" >> $log
diff 12345678.mgd77 dummy.mgd77 | tee -a fail >> $log
echo "Test if mgd77 from dat matches original:" >> $log
diff $$.mgd77 dummy.mgd77 | tee -a fail >> $log

rm -f $$.* 12345678.*

passfail convert
