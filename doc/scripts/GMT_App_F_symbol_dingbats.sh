#!/usr/bin/env bash
#
#	Makes the octal code charts in Appendix F
#
# Use the row, col values to generate the octal code needed and
# plot it with gmt pstext, including the header row and left column

gmt begin GMT_App_F_symbol_dingbats
gmt set MAP_FRAME_PEN thick FONT_TITLE 14p

# Chart for Symbols font
gmt set PS_CHAR_ENCODING ISOLatin1+

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
# plot it with gmt pstext, including the header row and left column

cat << EOF > tt.awk
BEGIN {
	printf "0.5 3.5 10p,4 octal\n"
	for (i = 0; i < 8; i++)
	{
		printf "%g 3.5 10p,4 %d\n", i + 1.5, i
	}
}
{
	printf "0.5 %g 10p,4 \\\\\\\%02ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%g %g 10p,12 \\\\%02o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

$AWK -f tt.awk tt.txt > tt.d
gmt plot -R0/9/3/16 -Jx0.345i/-0.21i -BN+tSymbol -Glightgreen -Y2.58i << EOF
>
8	16
9	16
9	15
8	15
EOF
gmt text tt.d -F+f
gmt plot -Bg1 -Wthick << EOF
>
0	4
9	4
>
1	3
1	16
EOF

cat << EOF > tt.txt
20	0	1	2	3	4	5	6	7
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
	printf "0.5 %g 10p,4 \\\\\\\%02ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%g %g 10p,12 \\\\%02o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

$AWK -f tt.awk tt.txt > tt.d
gmt plot -R0/9/20/32 -Glightgreen -Y-2.58i << EOF
#> The Euro symbol now goes here so I have commented out this green box
#1	21
#2	21
#2	20
#1	20
>
8	32
9	32
9	31
8	31
EOF
gmt text tt.d -F+f
gmt plot -Bg1 -Wthick << EOF
>
1	20
1	32
EOF


########################################################################################
# Now the dinghbats script
########################################################################################

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
# plot it with gmt pstext, including the header row and left column

cat << EOF > tt.awk
BEGIN {
	printf "0.5 3.5 10p,4 octal\n"
	for (i = 0; i < 8; i++)
	{
		printf "%g 3.5 10p,4 %d\n", i + 1.5, i
	}
}
{
	printf "0.5 %g 10p,4 \\\\\\\%02ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%g %g 10p,34 \\\\%02o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

$AWK -f tt.awk tt.txt > tt.d

# Chart for ZapfDingbats
gmt set PS_CHAR_ENCODING ISOLatin1+

gmt plot -R0/9/3/16 -Jx0.345i/-0.21i -BN+tZapfDingbats -Glightgreen -X3.2i -Y2.58i << EOF
>
8	16
9	16
9	15
8	15
EOF
gmt text tt.d -F+f
gmt plot -Bg1 -Wthick << EOF
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
	printf "0.5 %g 10p,4 \\\\\\\%02ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%g %g 10p,34 \\\\%02o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

$AWK -f tt.awk tt.txt > tt.d
gmt plot -R0/9/20/32 -Glightgreen -Y-2.58i << EOF
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
gmt text tt.d -F+f
gmt plot -Bg1 -Wthick << EOF
>
1	20
1	32
EOF
gmt end show
