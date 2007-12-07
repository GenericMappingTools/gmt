#!/bin/sh
#	$Id: shelltest.sh,v 1.1 2007-12-07 19:22:09 guru Exp $
#
# Testing gmt_shell_functions

. gmt_shell_functions.sh

gmt_message "Testing the GMT shell functions"

# Test record counter

cat << EOF > crap.txt
1 2
3 4
5 6
7 8
EOF

n=`gmt_nrecords crap.txt`
gmt_message "We got $n records"

while read line; do
	n=`gmt_nfields $line`
	i=1
	while [ $i -le $n ]; do
		x=`gmt_get_field $i "$line"`
		gmt_message "Field $i is $x"
		i=`expr $i + 1`
	done
done < crap.txt

R=`gmt_get_region crap.txt -I2`
gmt_message "Found the table region to be $R"

grdmath -R0/50/10/90 -I10 X Y MUL = crap.grd

R=`gmt_get_gridregion crap.grd`
gmt_message "Found the grid region to be $R"

PS=`gmt_set_psfile $0`

gmt_message "PS name is $PS"
