#!/usr/bin/env bash
# Ensure -W -N play nice, see https://github.com/GenericMappingTools/gmt/issues/5704

echo "0 0
1 1
0 1" > clip.txt
echo ".5 .25
.5 .5
.5 .75
.25 .5
.75 .5" > data.txt

gmt begin clip_draw
	# -N
	gmt subplot begin 2x2 -Fs8c -Scb -Srl -R0/1/0/1 -A
	gmt subplot set -A"-N"
	gmt basemap
	gmt clip clip.txt -N
	gmt plot -Sc.5c -G128 -W.5p,0 data.txt
	gmt clip -C
	# -N -W
	gmt subplot set -A"-N -W"
	gmt clip clip.txt -N -W.25p,blue
	gmt plot -Sc.5c -G128 -W.5p,0 data.txt
	gmt clip -C
	# neither
	gmt subplot set -A"none"
	gmt basemap
	gmt clip clip.txt
	gmt plot -Sc.5c -G128 -W.5p,0 data.txt
	gmt clip -C
	#-W
	gmt subplot set -A"-W"
	gmt psbasemap
	gmt clip clip.txt -W.25p,blue
	gmt plot -Sc.5c -G128 -W.5p,0 data.txt
	gmt clip -C
	gmt subplot end
gmt end show
