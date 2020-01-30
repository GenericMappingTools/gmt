#!/usr/bin/env bash
#
# Testing gmt_shell_functions

. gmt_shell_functions.sh

gmt_message "Testing the GMT shell functions"

gmt_init_tmpdir
gmt_message "Created temporary directory $GMT_TMPDIR"

# Test record counter

cat << EOF > $GMT_TMPDIR/crap.txt
1 2
3 4
5 6
7 8
EOF

n=$(gmt_nrecords $GMT_TMPDIR/crap.txt)
gmt_message "We got $n records"

rec=0
while read line; do
	rec=$(expr $rec + 1)
	n=$(gmt_nfields $line)
	i=1
	while [ $i -le $n ]; do
		x=$(gmt_get_field $i "$line")
		gmt_message "Rec $rec Field $i is $x"
		i=$(expr $i + 1)
	done
done < $GMT_TMPDIR/crap.txt

R=$(gmt_get_region $GMT_TMPDIR/crap.txt -I2)
gmt_message "Found the table region to be $R"

gmt grdmath -R0/50/10/90 -I10 X Y MUL = $GMT_TMPDIR/crap.nc

R=$(gmt_get_gridregion $GMT_TMPDIR/crap.nc)
gmt_message "Found the grid region to be $R"

PS=$(gmt_set_psfile $0)

gmt_message "PS name is $PS"

gmt_message "Removing temporary directory $GMT_TMPDIR"
gmt_remove_tmpdir
