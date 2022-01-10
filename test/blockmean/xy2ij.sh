#!/usr/bin/env bash
#
# Test to make sure the (x,y) <--> (i,j) macros work correctly
# We use -R0/5/0/5 -I1 for pixel and gridline registrations
# Points are prepared that will land in various tiles, some will
# fall exactly on tile boundaries.  One point will fall outside -R
# but should be considered for the gridline registrated case.
# DVC_TEST

ps=xy2ij.ps

# Set up the four answer files:
cat << EOF > pixel_ij_answer.txt
0.5	4.5	0
4.5	4.5	6
0.5	3.5	4
2.5	3.5	5
4.5	3.5	8
0.5	2.5	3
2.5	2.5	7
1.5	1.5	2
2.5	1.5	9
0.5	0.5	1
EOF
cat << EOF > pixel_xy_answer.txt
0	4.1	0
4.67	4	6
0.69	3.33	4
2	3.3	5
5	3.35	8
0.25	2.25	3
3	2	7
1.33	1.8	2
3	1.3	9
0.65	0.7	1
EOF
cat << EOF > pixel.txt
0.0	4.1	0
0.65	0.7	1
1.33	1.8	2
0.25	2.25	3
0.69	3.33	4
2.0	3.3	5
4.67	4.0	6
3.0	2.0	7
5.0	3.35	8
3.0	1.3	9
EOF
cat << EOF > grid_ij_answer.txt
0	4	0
1	4	4
2	4	5
4	4	6
1	3	3
2	3	9
4	2	7
2	1	2
1	0	1
EOF
cat << EOF > grid_xy_answer.txt
-0.5	4.1	0
1.19	3.83	4
2.5	3.8	5
4.167	3.5	6
0.75	2.75	3
1.5	2.8	9
3.5	2.5	7
1.83	1.3	2
1.15	0.2	1
EOF

# blockman uses GMT_x|y_to_i|j to determine which tiles
# Using -C gives tile centers via (i,j) -> (x,y)
gmt blockmean -R0/5/0/5 -I1 -r -C pixel.txt > pixel_ij.txt
gmt blockmean -R0/5/0/5 -I1 -r pixel.txt > pixel_xy.txt

# Do gridline registration with a similar data set (mostly offset by 0.5)
cat << EOF > grid.txt
-0.5	4.1	0
1.15	0.2	1
1.83	1.3	2
0.75	2.75	3
1.19	3.83	4
2.5	3.8	5
4.167	3.5	6
3.5	2.5	7
5.5	3.35	8
1.5	2.8	9
EOF
gmt blockmean -R0/5/0/5 -I1 -C grid.txt > grid_ij.txt
gmt blockmean -R0/5/0/5 -I1 grid.txt > grid_xy.txt

diff pixel_xy.txt pixel_xy_answer.txt --strip-trailing-cr  > fail
diff pixel_ij.txt pixel_ij_answer.txt --strip-trailing-cr >> fail
diff grid_xy.txt  grid_xy_answer.txt --strip-trailing-cr  >> fail
diff grid_ij.txt  grid_ij_answer.txt --strip-trailing-cr  >> fail

# Connect the original point and the corresponding tile center
paste pixel_xy.txt pixel_ij.txt | $AWK '{if (NF == 6) printf ">\n%s\t%s\n%s\t%s\n", $1, $2, $4, $5}' \
	| gmt psxy -R0/5/0/5 -JX4.5i -P -B1g1 -BWSne -Wdefault -K -Y0.5i -X2i > $ps
# Plot and label the points
gmt psxy -R -J pixel.txt -Sc0.125 -Gwhite -Wfaint -O -K -N >> $ps
gmt pstext -R -J pixel.txt -F+f8p -O -K -N >> $ps
gmt psxy -R -J pixel_ij.txt -Sc0.15 -Gblack -O -K -N >> $ps
gmt pstext -R -J pixel_ij.txt -F+f8p,white -O -K -N >> $ps

# Now do the same with gridline orientation
gmt psbasemap -R0.5/5.5/0.5/5.5 -J -O -Bg1 -K -Y4.7 >> $ps
paste grid_xy.txt grid_ij.txt | $AWK '{if (NF == 6) printf ">\n%s\t%s\n%s\t%s\n", $1, $2, $4, $5}' \
	| gmt psxy -R0/5/0/5 -J -O -B1 -BWsne -Wdefault -K >> $ps
gmt psxy -R -J grid.txt -Sc0.125 -Gwhite -Wfaint -O -K -N >> $ps
gmt pstext -R -J grid.txt -F+f8p -O -K -N >> $ps
gmt psxy -R -J grid_ij.txt -Sc0.15 -Gblack -O -K -N >> $ps
gmt pstext -R -J grid_ij.txt -F+f8p,white -O -N >> $ps
