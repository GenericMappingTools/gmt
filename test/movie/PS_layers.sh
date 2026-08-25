#!/usr/bin/env bash
#
# Test that movie works with two precomputed PS layers as fore- and back-grounds.
ps=PS_layers.ps
# Must ensure the external PS and the movie PS use the same char set
gmt set PS_CHAR_ENCODING ISOLatin1+
# Make the two dummy layers: a pink background rectangle and a foreground DRAFT tex
gmt plot -R0/9.4/0/5.2 -Jx1i -X0.1 -Y0.1 -T -B0 -B+gpink --PS_MEDIA=9.6ix5.4i -ps background
echo DRAFT | gmt text -R0/9.4/0/5.2 -Jx1i -X0.1 -Y0.1 -F+f200+cCM --PS_MEDIA=9.6ix5.4i -ps foreground
cat << EOF > map.sh
gmt begin
	gmt basemap -R0/10/0/5 -JX7.6i/3.5i -B
gmt end
EOF
# Just request the master PS frame
gmt movie map.sh -CHD -NPS_layers -T50 -Fnone -M10,ps -Sbbackground.ps -Sfforeground.ps
# Must delete the two layers since otherwise the test machinery will try to compare these PS
# files with originals which do not exist...
rm -f background.* foreground.*
