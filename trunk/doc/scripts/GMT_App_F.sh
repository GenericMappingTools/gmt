#!/bin/sh
#	$Id: GMT_App_F.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#
#	Makes the octal code chart in Appendix F


# Use the row, col values to generate the octal code needed and
# plot it with pstext, including the header row and left column

gmtset WANT_EURO_FONT TRUE FRAME_PEN 1p

# First chart for standard font

# First col is row number, the remaining cols are col number in table
# that has a printable character

cat << EOF > chart.d
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
15	0	1	2	3	4	5	6
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
31	0	1	2	3	4	5	6
EOF

# Use the row, col values to generate the octal code needed and
# plot it with pstext, including the header row and left column

cat << EOF > f.awk
BEGIN {
	printf "0.5 2.5 10 0 4 MC octal\n"
	for (i = 0; i < 8; i++)
	{
		printf "%lg 2.5 10 0 4 MC %d\n", i + 1.5, i
	}
}
{
	printf "0.5 %lg 10 0 4 MC \\\\\\\%2.2ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%lg %lg 10 0 4 MC \\\\%2.2o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

gawk -f f.awk chart.d > t
psxy -R0/9/2/32 -Jx0.345/-0.21 -B0g1 -P -K -M -G200 -Y0.0 << EOF > GMT_App_F_1.ps
>
1	4
2	4
2	3
1	3
>
8	16
9	16
9	15
8	15
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
pstext t -R -Jx -O -K >> GMT_App_F_1.ps
psxy -R -Jx -O -M -W1p << EOF >> GMT_App_F_1.ps
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

cat << EOF > chart.d
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

cat << EOF > f.awk
BEGIN {
	printf "0.5 3.5 10 0 4 MC octal\n"
	for (i = 0; i < 8; i++)
	{
		printf "%lg 3.5 10 0 4 MC %d\n", i + 1.5, i
	}
}
{
	printf "0.5 %lg 10 0 4 MC \\\\\\\%2.2ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%lg %lg 10 0 12 MC \\\\%2.2o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

gawk -f f.awk chart.d > t
psxy -R0/9/3/16 -Jx0.345/-0.21 -B0g1 -P -K -M -G200 -Y2.58 << EOF > GMT_App_F_2.ps
>
8	16
9	16
9	15
8	15
EOF
pstext t -R -Jx -O -K >> GMT_App_F_2.ps
psxy -R -Jx -O -K -M -W1p << EOF >> GMT_App_F_2.ps
>
0	4
9	4
>
1	3
1	16
EOF

cat << EOF > chart.d
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

cat << EOF > f.awk
{
	printf "0.5 %lg 10 0 4 MC \\\\\\\%2.2ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%lg %lg 10 0 12 MC \\\\%2.2o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

gawk -f f.awk chart.d > t
psxy -R0/9/20/32 -Jx -B0g1 -O -K -M -G200 -Y-2.58 << EOF >> GMT_App_F_2.ps
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
pstext t -R -Jx -B0g1 -O -K >> GMT_App_F_2.ps
psxy -R -Jx -O -M -W1p << EOF >> GMT_App_F_2.ps
>
0	21
9	21
>
1	20
1	32
EOF

# Then chart for ZapfDingbats

# First col is row number, the remaining cols are col number in table
# that has a printable character

cat << EOF > chart.d
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

cat << EOF > f.awk
BEGIN {
	printf "0.5 3.5 10 0 4 MC octal\n"
	for (i = 0; i < 8; i++)
	{
		printf "%lg 3.5 10 0 4 MC %d\n", i + 1.5, i
	}
}
{
	printf "0.5 %lg 10 0 4 MC \\\\\\\%2.2ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%lg %lg 10 0 34 MC \\\\%2.2o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

gawk -f f.awk chart.d > t
psxy -R0/9/3/16 -Jx0.345/-0.21 -B0g1 -P -K -M -G200 -Y2.58 << EOF > GMT_App_F_3.ps
>
8	16
9	16
9	15
8	15
EOF
pstext t -R -Jx -O -K >> GMT_App_F_3.ps
psxy -R -Jx -O -K -M -W1p << EOF >> GMT_App_F_3.ps
>
0	4
9	4
>
1	3
1	16
EOF

cat << EOF > chart.d
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

cat << EOF > f.awk
{
	printf "0.5 %lg 10 0 4 MC \\\\\\\%2.2ox\n", \$1+0.5, \$1
	for (i = 2; i <= NF; i++)
	{
		printf "%lg %lg 10 0 34 MC \\\\%2.2o%o\n", \$i+1.5, \$1+0.5, \$1, \$i
	}
}
EOF

gawk -f f.awk chart.d > t
psxy -R0/9/20/32 -Jx -B0g1 -O -K -M -G200 -Y-2.58 << EOF >> GMT_App_F_3.ps
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
pstext t -R -Jx -B0g1 -O -K >> GMT_App_F_3.ps
psxy -R -Jx -O -M -W1p << EOF >> GMT_App_F_3.ps
>
0	21
9	21
>
1	20
1	32
EOF

\rm -f chart.d f.awk t

gmtset FRAME_PEN 1.25p
