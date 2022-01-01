#!/usr/bin/env bash
#               GMT ANIMATION 01
#
# Purpose:      Make simple MP4 of sine function
# GMT modules:  math, basemap, text, plot, movie
# Unix progs:   echo, convert, cat
#
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/5m3gRhFFFLA

# 1. Create files needed in the loop
cat << 'EOF' > pre.sh
gmt math -T0/360/10 T SIND = sin_point.txt
gmt math -T0/360/1 T SIND = sin_curve.txt
gmt begin
	gmt basemap -R0/360/-1.2/1.6 -JX22c/11.5c -X1c -Y1c \
	-BWSne+glightskyblue -Bxa90g90f30+u@. -Bya0.5f0.1g1 --FONT_ANNOT_PRIMARY=9p
gmt end
EOF
# 2. Set up the main frame script
cat << 'EOF' > main.sh
gmt begin
#	Plot smooth blue curve and dark red dots at all angle steps so far
	last=$(gmt math -Q ${MOVIE_FRAME} 10 MUL =)
	gmt convert sin_curve.txt -qi0:${last} | gmt plot -W1p,blue -R0/360/-1.2/1.6 -JX22c/11.5c -X1c -Y1c
	gmt convert sin_point.txt -qi0:${MOVIE_FRAME} | gmt plot -Sc0.1i -Gdarkred
#	Plot bright red dot at current angle and annotate
	gmt plot -Sc0.1i -Gred <<< "${MOVIE_COL0} ${MOVIE_COL1}"
	printf "0 1.6 a = %3.3d" ${MOVIE_COL0} | gmt text -F+f14p,Helvetica-Bold+jTL -N -Dj0.1i/0.05i
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -Sbpre.sh -Chd -Tsin_point.txt -Vi -D5 -Zs -Nanim01 -Fmp4
