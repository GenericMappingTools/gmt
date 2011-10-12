#!/bin/bash
#
#	$Id$
#
#	This plots the given encoding vector to stdout
#
#	e.g., GMT_encoding.sh ISO-8859-1 | gv -
#
. ./functions.sh

if [ $# -eq 0 ]; then
	exit
fi

cat << EOF > $$.awk	# This awk script creates the $$.chart table of which entries are defined
{
	printf "%d\t", NR-1
	for (i = 1; i < 8; i++)
	{
		if (\$i != "/.notdef") printf "%d", i-1
		printf "\t"
	}
	if (\$8 != "/.notdef") printf "7"
	printf "\n"
}
EOF
egrep -v '\[|\]' ../../share/pslib/$1.ps | $AWK -f $$.awk > $$.chart
cat << EOF > $$.awk	# This awk script creates a file for psxy to plot a rectangle for undefined entries
{
	for (i = 1; i <= 8; i++)
	{
		if (\$i == "/.notdef") printf "%g %g 0.345 0.21\n", i + 0.5, NR-0.5
	}
}
EOF
egrep -v '\[|\]' ../../share/pslib/$1.ps | $AWK -f $$.awk > $$.empty

cat << EOF > $$.awk
BEGIN {
	printf "0.5 -0.5 octal\n"
	for (i = 0; i < 8; i++)
	{
		printf "%g -0.5 %d\n", i + 1.5, i
	}
}
{
	printf "0.5 %g \\\\\\\%2.2ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%g %g \\\\%2.2o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

gmtset PS_CHAR_ENCODING $1
psxy -R0/9/-1/32 -Jx0.345/-0.21 -Bg1:."Octal codes for $1": -P -K -Ggray -X3 -Sr $$.empty
$AWK -f $$.awk $$.chart | pstext -R -J -O -K -F+f10p,Times-Roman
psxy -R -J -O -Wthick << EOF
>
0	0
9	0
>
1	0
1	32
EOF
