#!/usr/bin/env bash
#
# GMT animations all start with designing plots that are created using the
# PostScript language.  It is therefore vector graphics with no limitations
# imposed by pixel resolutions.  However, to make an animation we must render
# these PostScript plots into raster images (we use PNG) and a pixel resolution
# enters.  Unlike printed media (laserwriters), the dots-per-unit in an animation
# is much lower, and compromizes are made when vector graphics must be turned
# into pixels.  GMT's movie module (and psconvert for still images) offers the
# option of sub-pixeling.  It means the image is temporarily enlarged to have
# more pixels than requested, then shrunk back down.  These steps tend to make
# the lower-resolution images better than the raw rendering.  Here we show
# the effect of different sub-pixel settings - notice how the movies with
# little or no sub-pixeling "jitters" as time goes by.
# The resulting movie was presented at the Fall 2019 AGU meeting in an eLighting talk:
# P. Wessel, 2019, GMT science animations for the masses, Abstract IN21B-11.
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/FLzYVo7wXAg
# The movie took ~2 minutes to render on a 24-core MacPro 2013.
# Demonstrate the effect of sub-pixeling
# 1. Create the angle file
cat << 'EOF' > pre.sh
gmt begin
	gmt math -T0/30/0.05 T 15 ADD = angles.txt
gmt end
EOF
# 2. Set up the main frame script
cat << 'EOF' > main.sh
gmt begin
	echo "BELL" | gmt text -R-26/-12/63/67 -JM6i -F+f144p+cCM -Bafg \
		-X2i -Y1.25i -p${MOVIE_COL0}/${MOVIE_COL1}+w20W/65N+v3i/1.5i
	echo -15 65 100 | gmt plot -SE- -Gred -p
	echo -25 66.5 -20 66.5  | gmt plot -SV0.5i+s+e+h0.5 -Gblue -W3p -p
gmt end
EOF
gmt movie main.sh -C540p -Njitter_H0 -Tangles.txt -Sbpre.sh -D24 -Fmp4 -G+p2p -Z -W -Lc0+jTL+t%05.1f+o1c -Ls"0 sub-pixels"+jBR+o1c
for H in 2 4 8; do
	gmt movie main.sh -C540p -Njitter_H${H} -Tangles.txt -Sbpre.sh -D24 -Fmp4 -G+p2p -H${H} -Z -W -Lc0+jTL+t%05.1f+o1c -Ls"${H} sub-pixels"+jBR+o1c
done
# 4. Assemble the four movie frames into a 2x2 HD layout via ffmpeg
ffmpeg -loglevel warning -i jitter_H0.mp4 -i jitter_H2.mp4 -filter_complex hstack=inputs=2 top.mp4
ffmpeg -loglevel warning -i jitter_H4.mp4 -i jitter_H8.mp4 -filter_complex hstack=inputs=2 bot.mp4
ffmpeg -loglevel warning -i top.mp4 -i bot.mp4 -filter_complex vstack=inputs=2 anim10.mp4
rm -f top.mp4 bot.mp4 jitter_H?.mp4 main.sh pre.sh angles.txt
