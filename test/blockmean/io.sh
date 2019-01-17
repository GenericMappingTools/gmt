#!/usr/bin/env bash
#
# Test that the -i, -o, -bi and bo works OK.

# 0. create the file with correct answers:
cat << EOF > io_answer.txt
0	0	0
0	0	0
0	0
0	0
EOF

# 1. Prepare input files with 8 columns, both ASCII and binary
cat << EOF > ascii_i.txt
0	4.1	0	0	4.1	0	41 	1
3	4.3	9	3	1.3	90	13 	91
4.67	4	6	4.67	4	60	4 	61
0.69	3.33	4	0.69	3.33	40	33	41
2	3.3	5	2	3.3	50	33 	51
5	3.35	8	5	3.35	80	35	81
0.25	2.25	3	0.25	2.25	30	25	31
3	2	7	3	2	70	2	71
1.33	1.8	2	1.33	1.8	20	18 	21
0.65	0.7	1	0.65	0.7	10	7 	11
EOF
gmt convert ascii_i.txt -bo8d > bin_i.b

# 2. do basic gmt blockmean ASCII/bin i/o with no -i/-o
gmt blockmean -R0/5/0/5 -I1 -r ascii_i.txt > ascii_o.txt
gmt blockmean -R0/5/0/5 -I1 -r bin_i.b -bi8d -bo3d | gmt convert -bi3d > bin_o.txt
gmt math -T -Sl ascii_o.txt bin_o.txt SUB SUM = io_result.txt

# 3. Same as 2, but with selecting cols 3:5 via -i
gmt blockmean -R0/5/0/5 -I1 -r ascii_i.txt -i3:5 > ascii_o.txt
gmt blockmean -R0/5/0/5 -I1 -r bin_i.b -bi8d -i3:5 -bo3d | gmt convert -bi3d > bin_o.txt
gmt math -T -Sl ascii_o.txt bin_o.txt SUB SUM = >> io_result.txt

# 4. Same 2-3, but just output cols 2,0 via -o
gmt blockmean -R0/5/0/5 -I1 -r ascii_i.txt -o2,0 > ascii_o.txt
gmt blockmean -R0/5/0/5 -I1 -r bin_i.b -bi8d -o2,0 -bo2d | gmt convert -bi2d > bin_o.txt
gmt math -T -Sl ascii_o.txt bin_o.txt SUB SUM = >> io_result.txt

# 5. Same 5, but with selecting cols 3:5 via -i and output cols 2,0 via -o
gmt blockmean -R0/5/0/5 -I1 -r ascii_i.txt -i3:5 -o2,0 > ascii_o.txt
gmt blockmean -R0/5/0/5 -I1 -r bin_i.b -bi8d -i3:5 -o2,0 -bo2d | gmt convert -bi2d > bin_o.txt
gmt math -T -Sl ascii_o.txt bin_o.txt SUB SUM = >> io_result.txt

diff io_result.txt io_answer.txt --strip-trailing-cr > fail
#rm -f ascii_i.txt ascii_o.txt bin_o.txt bin_i.b
