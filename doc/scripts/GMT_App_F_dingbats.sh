#!/bin/bash
#	$Id$
#
#	Makes the octal code charts in Appendix F
. ./functions.sh

# Use the row, col values to generate the octal code needed and
# plot it with pstext, including the header row and left column

gmtset MAP_FRAME_PEN thick FONT_TITLE 14p

# First col is row number, the remaining cols are col number in table
# that has a printable character

cat << EOF > tt.txt
4	0	1	2	3	4	5	6	7
5	0	1	2	3	4	5	6	7
6	0	1	2	3	4	5	6	7
7	0	1	2	3	4	5	6	7
8	0	1	2	3	4	5	6	7
9	0	1	2	3	4	5	6	7
10	0	1	2	3	4	5	6	7
11	0	1	2	3	4	5	6	7
12	0	1	2	3	4	5	6	7
13	0	1	2	3	4	5	6	7
14	0	1	2	3	4	5	6	7
15	0	1	2	3	4	5	6
EOF

# Use the row, col values to generate the octal code needed and
# plot it with pstext, including the header row and left column

cat << EOF > tt.awk
BEGIN {
	printf "0.5 3.5 10p,4 octal\n"
	for (i = 0; i < 8; i++)
	{
		printf "%g 3.5 10p,4 %d\n", i + 1.5, i
	}
}
{
	printf "0.5 %g 10p,4 \\\\\\\%2.2ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%g %g 10p,34 \\\\%2.2o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

$AWK -f tt.awk tt.txt > tt.d

# Chart for ZapfDingbats
gmtset PS_CHAR_ENCODING ISOLatin1+

psxy -R0/9/3/16 -Jx0.345i/-0.21i -B:.ZapfDingbats:N -P -K -Glightgreen -Y2.58i << EOF > GMT_App_F_dingbats.ps
>
8	16
9	16
9	15
8	15
EOF
pstext tt.d -R -J -O -K -F+f >> GMT_App_F_dingbats.ps
psxy -R -J -O -Bg1 -K -Wthick << EOF >> GMT_App_F_dingbats.ps
>
0	4
9	4
>
1	3
1	16
EOF

cat << EOF > tt.txt
20		1	2	3	4	5	6	7
21	0	1	2	3	4	5	6	7
22	0	1	2	3	4	5	6	7
23	0	1	2	3	4	5	6	7
24	0	1	2	3	4	5	6	7
25	0	1	2	3	4	5	6	7
26	0	1	2	3	4	5	6	7
27	0	1	2	3	4	5	6	7
28	0	1	2	3	4	5	6	7
29	0	1	2	3	4	5	6	7
30	0	1	2	3	4	5	6	7
31	0	1	2	3	4	5	6
EOF

cat << EOF > tt.awk
{
	printf "0.5 %g 10p,4 \\\\\\\%2.2ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%g %g 10p,34 \\\\%2.2o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

$AWK -f tt.awk tt.txt > tt.d
psxy -R0/9/20/32 -J -O -K -Glightgreen -Y-2.58i << EOF >> GMT_App_F_dingbats.ps
>
1	21
2	21
2	20
1	20
>
8	32
9	32
9	31
8	31
EOF
pstext tt.d -R -J -O -K -F+f >> GMT_App_F_dingbats.ps
psxy -R -J -O -Bg1 -Wthick << EOF >> GMT_App_F_dingbats.ps
>
1	20
1	32
EOF
