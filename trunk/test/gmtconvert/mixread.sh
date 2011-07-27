#!/bin/bash
#	$Id$
# Test gmtconvert with mixed format binary input

. ../functions.sh
header "Let gmtconvert convert mixed binary records"

# This is what the output should look like
cat << EOF >> $$.d
3.1400001	3.1400001	3.14	3.14	3.14	9999	9999	123123.000000	123123
3.1400001	3.1400001	3.14	3.14	3.14	9999	9999	123123.000000	123123
3.1400001	3.1400001	3.14	3.14	3.14	9999	9999	123123.000000	123123
EOF
# Use +L since binary file was created on a little-endian OS X box
gmtconvert -bi2f,3d,2h,10x,2i+L mix_binary_data.b --FORMAT_FLOAT_OUT=2-4:%.2f,7:%12.6f,%.8g > $$.txt
diff $$.d $$.txt --strip-trailing-cr > fail
if [ ! -s fail ]; then
	passfail mixread
else
	cat fail
fi
rm -f $$.*
