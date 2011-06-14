#!/bin/bash
#	$Id: GMT_App_F.sh,v 1.21 2011-06-14 03:02:54 remko Exp $
#
#	Makes the octal code charts in Appendix F
. ./functions.sh

# Use the row, col values to generate the octal code needed and
# plot it with pstext, including the header row and left column

gmtset PS_CHAR_ENCODING Standard MAP_FRAME_PEN thick FONT_TITLE 14p

# First chart for standard font

# First col is row number, the remaining cols are col number in table
# that has a printable character

cat << EOF > $$.txt
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
# plot it with pstext, including the header row and left column

cat << EOF > $$.awk
BEGIN {
	printf "0.5 2.5 10p,4 octal\n"
	for (i = 0; i < 8; i++)
	{
		printf "%g 2.5 10p,4 %d\n", i + 1.5, i
	}
}
{
	printf "0.5 %g 10p,4 \\\\\\\%2.2ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%g %g 10p,4 \\\\%2.2o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

$AWK -f $$.awk $$.txt > $$.d
gmtset PS_CHAR_ENCODING Standard+
# Then for Standard+
# First mark uncoded entries
psxy -R0/9/2/32 -Jx0.345i/-0.21i -B:.Standard+:N -P -K -Glightred -Y0.0 << EOF > GMT_App_F_stand+.ps
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
#Then highlight Standard+ enhancements
psxy -R -J -O -K -Glightgreen << EOF >> GMT_App_F_stand+.ps
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
1	20
9	20
9	16
1	16
>
1	23
2	23
2	22
1	22
>
6	23
7	23
7	22
6	22
>
7	24
8	24
8	23
7	23
>
1	25
2	25
2	24
1	24
>
2	26
3	26
3	25
2	25
>
5	26
6	26
6	25
5	25
>
2	27
9	27
9	26
2	26
>
1	28
9	28
9	27
1	27
>
1	29
2	29
2	28
1	28
>
3	29
4	29
4	28
3	28
>
5	29
9	29
9	28
5	28
>
1	30
2	30
2	29
1	29
>
5	30
9	30
9	29
5	29
>
1	31
2	31
2	30
1	30
>
3	31
6	31
6	30
3	30
>
7	31
9	31
9	30
7	30
>
1	32
2	32
2	31
1	31
>
5	32
9	32
9	31
5	31
EOF
pstext $$.d -R -J -O -K -F+f >> GMT_App_F_stand+.ps
psxy -R -J -O -Bg1 -Wthick << EOF >> GMT_App_F_stand+.ps
>
0	3
9	3
>
1	2
1	32
EOF


gmtset PS_CHAR_ENCODING ISOLatin1+
# Then for ISOLatin1+
# First the uncoded ones
psxy -R0/9/2/32 -Jx0.345i/-0.21i -B:.ISOLatin1+:N -P -K -Glightred -Y0.0 << EOF > GMT_App_F_iso+.ps
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
psxy -R -J -O -K -Glightgreen << EOF >> GMT_App_F_iso+.ps
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
pstext $$.d -R -J -O -K -F+f >> GMT_App_F_iso+.ps
psxy -R -J -O -Bg1 -Wthick << EOF >> GMT_App_F_iso+.ps
>
0	3
9	3
>
1	2
1	32
EOF

# Then chart for Symbols font

# First col is row number, the remaining cols are col number in table
# that has a printable character

cat << EOF > $$.txt
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

cat << EOF > $$.awk
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
		printf "%g %g 10p,12 \\\\%2.2o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

$AWK -f $$.awk $$.txt > $$.d
psxy -R0/9/3/16 -Jx0.345i/-0.21i -B:.Symbol:N -P -K -Glightgreen -Y2.58i << EOF > GMT_App_F_symbol.ps
>
8	16
9	16
9	15
8	15
EOF
pstext $$.d -R -J -O -K -F+f >> GMT_App_F_symbol.ps
psxy -R -J -O -K -Bg1 -Wthick << EOF >> GMT_App_F_symbol.ps
>
0	4
9	4
>
1	3
1	16
EOF

cat << EOF > $$.txt
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

cat << EOF > $$.awk
{
	printf "0.5 %g 10p,4 \\\\\\\%2.2ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%g %g 10p,12 \\\\%2.2o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

$AWK -f $$.awk $$.txt > $$.d
psxy -R0/9/20/32 -J -O -K -Glightgreen -Y-2.58i << EOF >> GMT_App_F_symbol.ps
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
pstext $$.d -R -J -O -K -F+f >> GMT_App_F_symbol.ps
psxy -R -J -O -Bg1 -Wthick << EOF >> GMT_App_F_symbol.ps
>
1	20
1	32
EOF

# Then chart for ZapfDingbats

# First col is row number, the remaining cols are col number in table
# that has a printable character

cat << EOF > $$.txt
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

cat << EOF > $$.awk
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

$AWK -f $$.awk $$.txt > $$.d
psxy -R0/9/3/16 -Jx0.345i/-0.21i -B:.ZapfDingbats:N -P -K -Glightgreen -Y2.58i << EOF > GMT_App_F_dingbats.ps
>
8	16
9	16
9	15
8	15
EOF
pstext $$.d -R -J -O -K -F+f >> GMT_App_F_dingbats.ps
psxy -R -J -O -Bg1 -K -Wthick << EOF >> GMT_App_F_dingbats.ps
>
0	4
9	4
>
1	3
1	16
EOF

cat << EOF > $$.txt
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

cat << EOF > $$.awk
{
	printf "0.5 %g 10p,4 \\\\\\\%2.2ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%g %g 10p,34 \\\\%2.2o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

$AWK -f $$.awk $$.txt > $$.d
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
pstext $$.d -R -J -O -K -F+f >> GMT_App_F_dingbats.ps
psxy -R -J -O -Bg1 -Wthick << EOF >> GMT_App_F_dingbats.ps
>
1	20
1	32
EOF
