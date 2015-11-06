#!/bin/bash
#	$Id$
# Test gmt gmtconvert with mixed format binary input

# This is what the output should look like
cat << EOF >> tt.d
3.1400001	3.1400001	3.14	3.14	3.14	9999	9999	123123.000000	123123
3.1400001	3.1400001	3.14	3.14	3.14	9999	9999	123123.000000	123123
3.1400001	3.1400001	3.14	3.14	3.14	9999	9999	123123.000000	123123
EOF
# Use +L since binary file was created on a little-endian OS X box
gmt gmtconvert -bi2f,3d,2h,10x,2i+L mix_binary_data.b --FORMAT_FLOAT_OUT=0-1:%.8g,2-4:%.2f,5-6:%g,7:%12.6f,8:%g > tt.txt
diff tt.d tt.txt --strip-trailing-cr > fail
