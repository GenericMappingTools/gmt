#!/bin/bash
#
#	$Id$
#
#	This plots the given encoding vector to stdout
#
#	e.g., GMT_encoding.sh ISO-8859-1 | gv -
#	Dimensions are in inches.

if [ $# -eq 0 ]; then
	exit
fi

cat << EOF > tt.awk
# This awk script creates the tt.chart table of which entries are defined
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
egrep -v '\[|\]' "${GMT_SHAREDIR}"/share/postscriptlight/$1.ps | $AWK -f tt.awk > tt.chart
cat << EOF > tt.awk
# This awk script creates a file for gmt psxy to plot a rectangle for undefined entries
{
	for (i = 1; i <= 8; i++)
	{
		if (\$i == "/.notdef") printf "%g %g 0.345 0.21\n", i + 0.5, NR-0.5
	}
}
EOF
egrep -v '\[|\]' "${GMT_SHAREDIR}"/share/postscriptlight/$1.ps | $AWK -f tt.awk > tt.empty

cat << EOF > tt.awk
BEGIN {
	printf "0.5 -0.5 octal\n"
	for (i = 0; i < 8; i++)
	{
		printf "%g -0.5 %d\n", i + 1.5, i
	}
}
{
	printf "0.5 %g \\\\\\\%02ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%g %g \\\\%02o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

gmt gmtset PS_CHAR_ENCODING $1
gmt psxy -R0/9/-1/32 -Jx0.345i/-0.21i -Bg1 -B+t"Octal codes for $1" -P -K -Ggray -X3i -Sri tt.empty
$AWK -f tt.awk tt.chart | gmt pstext -R -J -O -K -F+f10p,Times-Roman
gmt psxy -R -J -O -Wthick << EOF
>
0	0
9	0
>
1	0
1	32
EOF
