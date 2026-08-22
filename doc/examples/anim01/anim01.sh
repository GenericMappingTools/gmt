#!/usr/bin/env bash
#               GMT ANIMATION 01
#
# Purpose:      Make simple MP4 of sine function
# GMT modules:  math, basemap, text, plot, movie, event
# Unix progs:   cat
#
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/NjSDpQ5S3FM

# 1. Create files needed in the loop
cat << 'EOF' > pre.sh
gmt math -T0/360/1 T SIND = sin_curve.txt
gmt begin
	gmt basemap -R0/360/-1.2/1.6 -JX22c/11.5c -X1c -Y1c \
	-BWSne+glightskyblue -Bxa90g90f30+u@. -Bya0.5f0.1g1 --FONT_ANNOT_PRIMARY=9p
gmt end
EOF
# 2. Set up the main frame script
cat << 'EOF' > main.sh
gmt begin
	gmt basemap -R0/360/-1.2/1.6 -JX22c/11.5c -X1c -Y1c -B+n
#	Plot smooth blue curve and dark red dots at all angle steps so far
	gmt events sin_curve.txt -i0,1,0 -T${MOVIE_FRAME} -Ar -Es -W1p,blue
	gmt plot sin_curve.txt -Sc0.25c -Gdarkred -qi0:10:${MOVIE_FRAME}

#	Plot bright red dot at current angle
	gmt events sin_curve.txt -T${MOVIE_FRAME} -Sc0.1i -Gred -i0,1,0 -L0
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -Sbpre.sh -Chd -Tsin_curve.txt -Vi -D50 -Zs -Nanim01 -Fmp4 \
	-Lf+t"a = %3.3d"+f14p,Helvetica-Bold+jTL+o1.25/1.15