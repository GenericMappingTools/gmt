#!/usr/bin/env bash
# Test movie options -E and -K
# Total of 19 separate tests or combinations
#
# 1 Set up title
cat << EOF > t.sh
gmt begin
	echo TITLE | gmt text -R0/5/0/5 -JX5i -B+glightpink -F+f24p+cCM -X0 -Y0
gmt end
EOF
# 2. Set up the main frame script
cat << EOF > m.sh
gmt begin
	gmt basemap -R0/5/0/5 -JX5i -Baf -B+glightgreen --MAP_FRAME_TYPE=inside -X0 -Y0
gmt end
EOF
# A. Run the movie: title plus plot, no fading anywhere
gmt movie m.sh -C5ix5ix100 -T48 -NA -D12 -Fmp4 -Et.sh+d24 -Lf -Z
# B. Run the movie: title plus plot, 6 frames fade in on title
gmt movie m.sh -C5ix5ix100 -T48 -NB -D12 -Fmp4 -Et.sh+d24+fi6 -Lf -Z
# C. Run the movie: title plus plot, 6 frames fade out on title
gmt movie m.sh -C5ix5ix100 -T48 -NC -D12 -Fmp4 -Et.sh+d24+fo6 -Lf -Z
# D. Run the movie: title plus plot, 6 frames fade in and out on title
gmt movie m.sh -C5ix5ix100 -T48 -ND -D12 -Fmp4 -Et.sh+d24+f6 -Lf -Z
# E. Run the movie: title plus plot, 6 frames fade in, 3 frames out on title
gmt movie m.sh -C5ix5ix100 -T48 -NE -D12 -Fmp4 -Et.sh+d24+fi6+fo3 -Lf -Z
# F. Run the movie: title plus plot, fade in on plot
gmt movie m.sh -C5ix5ix100 -T48 -NF -D12 -Fmp4 -Et.sh+d24 -K+fi6 -Lf -Z
# G. Run the movie: title plus plot, fade out on plot
gmt movie m.sh -C5ix5ix100 -T48 -NG -D12 -Fmp4 -Et.sh+d24 -K+fo6 -Lf -Z
# H. Run the movie: title plus plot, fade in/out on plot
gmt movie m.sh -C5ix5ix100 -T48 -NH -D12 -Fmp4 -Et.sh+d24 -K+f6 -Lf -Z
# I. Run the movie: title plus plot, fade in/out on plot unequally
gmt movie m.sh -C5ix5ix100 -T48 -NI -D12 -Fmp4 -Et.sh+d24 -K+fi6+fo12 -Lf -Z
# J. Run the movie: title plus plot, fade in title, fade out on plot 
gmt movie m.sh -C5ix5ix100 -T48 -NJ -D12 -Fmp4 -Et.sh+d24+fi6 -K+fo6 -Lf -Z
# K. Run the movie: title plus plot, fade in title, fade out in/on plot 
gmt movie m.sh -C5ix5ix100 -T48 -NK -D12 -Fmp4 -Et.sh+d24+fi6 -K+f6 -Lf -Z
# L. Run the movie: title plus plot, fade in/out title, fade in/out plot 
gmt movie m.sh -C5ix5ix100 -T48 -NL -D12 -Fmp4 -Et.sh+d24+f6 -K+f6 -Lf -Z
# M. Run the movie: title plus plot, no title fade, fade in/out plot with preserve both ends
gmt movie m.sh -C5ix5ix100 -T48 -NM -D12 -Fmp4 -Et.sh+d24 -K+f6+p -Lf -Z
# N. Run the movie: title plus plot, no title fade, fade in/out plot with preserve in
gmt movie m.sh -C5ix5ix100 -T48 -NN -D12 -Fmp4 -Et.sh+d24 -K+f6+pi -Lf -Z
# O. Run the movie: title plus plot, no title fade, fade in/out plot with preserve out
gmt movie m.sh -C5ix5ix100 -T48 -NO -D12 -Fmp4 -Et.sh+d24 -K+f6+po -Lf -Z
rm -f t.sh m.sh
