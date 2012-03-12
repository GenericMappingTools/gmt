#!/bin/bash
#	$Id$
#
# Test to make sure the (x,y) <--> (i,j) macros work correctly
# We use -R0/5/0/5 -I1 for pixel and gridline registrations
# Points are prepared that will land in various tiles, some will
# fall exactly on tile boundaries.  One point will fall outside -R
# but should be considered for the gridline registrated case.

. functions.sh
header "Test blockmean's (x,y) <--> (i,j) conversions (numerical)"

ps=xy2ij.ps
cat << EOF > pixel.d
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
# blockman uses GMT_x|y_to_i|j to determine which tiles
# Using -C gives tile centers via (i,j) -> (x,y)
blockmean -R0/5/0/5 -I1 -r -C pixel.d > pixel_ij.d
blockmean -R0/5/0/5 -I1 -r pixel.d > pixel_xy.d

# Do gridline registration with a similar data set (mostly offset by 0.5)
cat << EOF > grid.d
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
blockmean -R0/5/0/5 -I1 -C grid.d > grid_ij.d
blockmean -R0/5/0/5 -I1 grid.d > grid_xy.d

diff pixel_xy.d $src/pixel_xy.d --strip-trailing-cr  > fail
diff pixel_ij.d $src/pixel_ij.d --strip-trailing-cr >> fail
diff grid_xy.d  $src/grid_xy.d --strip-trailing-cr  >> fail
diff grid_ij.d  $src/grid_ij.d --strip-trailing-cr  >> fail

passfail xy2ij_numerical

header "Test blockmean's (x,y) <--> (i,j) conversions (plot)"

# Connect the original point and the corresponding tile center
paste pixel_xy.d pixel_ij.d | awk '{if (NF == 6) printf ">\n%s\t%s\n%s\t%s\n", $1, $2, $4, $5}' \
	| psxy -R0/5/0/5 -JX4.5 -P -B1g1WSne -Wdefault -K -Y0.5i -X2i > $ps
# Plot and label the points
psxy -R -J pixel.d -Sc0.125 -Gwhite -Wfaint -O -K -N >> $ps
pstext -R -J pixel.d -F+f8p -O -K -N >> $ps
psxy -R -J pixel_ij.d -Sc0.15 -Gblack -O -K -N >> $ps
pstext -R -J pixel_ij.d -F+f8p,white -O -K -N >> $ps

# Now do the same with gridline orientation
psbasemap -R0.5/5.5/0.5/5.5 -J -O -Bg1 -K -Y4.7 >> $ps
paste grid_xy.d grid_ij.d | awk '{if (NF == 6) printf ">\n%s\t%s\n%s\t%s\n", $1, $2, $4, $5}' \
	| psxy -R0/5/0/5 -J -O -B1Wsne -Wdefault -K >> $ps
psxy -R -J grid.d -Sc0.125 -Gwhite -Wfaint -O -K -N >> $ps
pstext -R -J grid.d -F+f8p -O -K -N >> $ps
psxy -R -J grid_ij.d -Sc0.15 -Gblack -O -K -N >> $ps
pstext -R -J grid_ij.d -F+f8p,white -O -N >> $ps

pscmp
