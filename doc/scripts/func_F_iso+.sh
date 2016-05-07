#!/bin/bash
#	$Id: GMT_App_F_iso+.sh 11490 2013-05-16 06:26:21Z pwessel $
#
#	Function that makes the octal code charts in Appendix F
#	but writes to stdout
#
# Use the row, col values to generate the octal code needed and
# plot it with gmt pstext, including the header row and left column

gmt gmtset MAP_FRAME_PEN thick FONT_TITLE 14p

# First chart for standard font

# First col is row number, the remaining cols are col number in table
# that has a printable character

cat << EOF > tt.txt
3		1	2	3	4	5	6	7
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
15	0	1	2	3	4	5	6	7
16	0	1	2	3	4	5	6	7
17	0	1	2	3	4	5	6	7
18	0	1	2	3	4	5	6	7
19	0	1	2	3	4	5	6	7
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
31	0	1	2	3	4	5	6	7
EOF

# Use the row, col values to generate the octal code needed and
# plot it with gmt pstext, including the header row and left column

cat << EOF > tt.awk
BEGIN {
	printf "0.5 2.5 10p,4 octal\n"
	for (i = 0; i < 8; i++)
	{
		printf "%g 2.5 10p,4 %d\n", i + 1.5, i
	}
}
{
	printf "0.5 %g 10p,4 \\\\\\\%02ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%g %g 10p,4 \\\\%02o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

$AWK -f tt.awk tt.txt > tt.d

# Chart ISOLatin1+ font
gmt gmtset PS_CHAR_ENCODING ISOLatin1+

# First the uncoded ones
gmt psxy -R0/9/2/32 -Jx0.345i/-0.21i -BN+tISOLatin1++ -P -K -Glightred -Y0.0 << EOF
>
1	4
2	4
2	3
1	3
>
1	21
2	21
2	20
1	20
EOF
# Then highlight ISOLatin1+ enhancements
gmt psxy -R -J -O -K -Glightgreen << EOF
>
2	4
9	4
9	3
2	3
>
8	16
9	16
9	15
8	15
>
1	18
9	18
9	16
1	16
>
2	20
3	20
3	19
2	19
>
5	20
6	20
6	19
5	19
EOF
gmt pstext tt.d -R -J -O -K -F+f
gmt psxy -R -J -O -Bg1 -Wthick << EOF
>
0	3
9	3
>
1	2
1	32
EOF
