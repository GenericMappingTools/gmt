#!/bin/bash
#       $Id$
# Testing gmtregress on the data in York [1966]
# York, D. (1966), Least-squares fitting of a straight line,
#    Can. J. Phys., 44, 1079-1086.

# Cols 3 and 4 are weights (=1/sigma)

ps=york_1966.ps
cat << EOF > york.txt
0	5.9	1e3	1
0.9	5.4	1e3	1.8
1.8	4.4	5e2	4
2.6	4.6	8e2	8
3.3	3.5	2e2	2e1
4.4	3.7	8e1	2e1
5.2	2.8	6e1	7e1
6.1	2.8	2e1	7e1
6.5	2.4	1.8	1e2
7.4	1.5	1	5e2
EOF

# First do LS with regression onto x or y only, with or without weights
# Invert table to recover sigmas so we may plot data with errorbars
awk '{print $1, $2, 1/$3, 1/$4}' york.txt > t.txt
gmt psxy -R-1/9/1/7 -Jx0.7i -P -K -Baf -BWSne -Sc0.03i -Gblue -Exy t.txt -X0.85i > $ps
# Plot LSY solution ignoring data errors
gmt regress -Ey -Fxm -N2 york.txt -T-0.5/8.5/8 | gmt psxy -R -J -O -K -W0.5p,red,- >> $ps
# Plot LSX solution ignoring data errors
gmt regress -Ex -Fxm -N2 york.txt -T-0.5/8.5/8 | gmt psxy -R -J -O -K -W0.5p,black,. >> $ps
# Plot LSY solution using y-errors
gmt regress -Ey -Fxm -N2 -Wwy york.txt -i0,1,3 -T-0.5/8.5/8 | gmt psxy -R -J -O -K -W1p,red >> $ps
# Plot LSX solution using x-errors
gmt regress -Ex -Fxm -N2 -Wwx york.txt -i0:2 -T-0.5/8.5/8 | gmt psxy -R -J -O -K -W0.25p,black >> $ps
gmt pslegend -DjTR+w1.9i+jRT+o0.1i/0.1i -R -J -O -K -F+p1p << EOF >> $ps
S 0.3i - 0.5i - 1p,red       0.6i LSY using @~s@~@-y@-
S 0.3i - 0.5i - 0.5p,red,-   0.6i LSY ignoring @~s@~@-y@-
S 0.3i - 0.5i - 0.25p,black  0.6i LSX using @~s@~@-x@-
S 0.3i - 0.5i - 0.5p,black,. 0.6i LSX ignoring @~s@~@-x@-
EOF

# Now do orthogonal LS with or without weights
gmt psxy -R -J -O -K -Baf -BWsne+t"York [1966] Regression" -Sc0.03i -Gblue -Exy t.txt -Y4.6i >> $ps
# Plot LSXY solution ignoring data errors
gmt regress -Eo -Fxm -N2 york.txt -T-0.5/8.5/8 | gmt psxy -R -J -O -K -W0.5p,red,- >> $ps
# Plot LSXY solution using both x- and y-errors
gmt regress -Eo -Fxm -N2 -Wwxy york.txt -T-0.5/8.5/8 | gmt psxy -R -J -O -K -W1p,red >> $ps
gmt pslegend -DjTR+w2.55i+jRT+o0.1i/0.1i -R -J -O -F+p1p << EOF >> $ps
S 0.3i - 0.5i - 1p,red       0.6i LSXY using @~s@~@-y@- and @~s@~@-x@-
S 0.3i - 0.5i - 0.5p,red,-   0.6i LSXY ignoring @~s@~@-y@- and @~s@~@-x@-
EOF
