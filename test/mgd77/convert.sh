#!/bin/sh
#	$Id: convert.sh,v 1.4 2007-07-04 14:05:44 remko Exp $
#
# Tests mgd77convert

echo -n "$0: Test mgd77convert conversions between mgd77-dat-nc:		"
MGD77_HOME=${MGD77_HOME:-$GMTHOME/share/mgd77}
export MGD77_HOME
cp dummy.mgd77 12345678.mgd77

# Make cdf file
mgd77convert 12345678 -Fa -Tc
# Make dat file
mgd77convert 12345678 -Fa -Tt

echo "Test if dat files are the same if coming from mgd77 or nc:" > log
mv 12345678.dat $$.dat
mgd77convert 12345678 -Fc -Tt
diff 12345678.dat $$.dat > fail
cat fail >> log
echo "Test if mgd77 files are the same if coming from nc or dat:" >> log
rm -f 12345678.mgd77
mgd77convert 12345678 -Ft -Ta
mv 12345678.mgd77 $$.mgd77
mgd77convert 12345678 -Fc -Ta
diff 12345678.mgd77 $$.mgd77 > $$.log
cat $$.log >> log
cat $$.log >> fail
echo "Test if mgd77 from nc matches original:" >> log
diff 12345678.mgd77 dummy.mgd77 > $$.log
cat $$.log >> log
cat $$.log >> fail
echo "Test if mgd77 from dat matches original:" >> log
diff $$.mgd77 dummy.mgd77 > $$.log
cat $$.log >> log
cat $$.log >> fail
if [ -s fail ]; then
        echo "[FAIL]"
	echo $0 >> ../fail_count.d
else
        echo "[PASS]"
        rm -f log
fi
rm -f $$.* 12345678.* fail
