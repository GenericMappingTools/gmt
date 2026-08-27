#!/usr/bin/env bash
#
# A 3-D panel must fit inside the subplot figure area.  This is the example from issue #4450.
#
# subplot sizes and places its panels from the nominal 2-D map rectangle, but a perspective panel is
# drawn as a projected box that is larger than that rectangle, and the box was never fitted to the
# panel: both cubes burst out of the -Ff20c/10c figure area, and the heading landed among them.
# gmt_map_setup now measures the projected footprint once the perspective is known and shrinks and
# centers it inside the panel.
#
# No baseline PostScript is needed: we measure the ink of the two perspective panels straight out of
# the PostScript and require it to fit the 10 cm height of the figure.

# This test measures the plot instead of comparing it, so it has no baseline PostScript.  gmtest gives
# any script that spells out the modern mode start command a $ps to compare, so we spell it via $start.
start=begin

gmt $start fit3d ps
  gmt subplot begin 1x2 -Ff20c/10c+pblack+wblue -T"3D Subplots"
    gmt subplot set 0,0
    gmt basemap -BneSWZ+b -R0/10/20/30/40/50 -Jz1 -p157.7/45

    gmt subplot set 0,1
    gmt basemap -BSEnwZ2+b -R0/10/20/30/40/50 -Jz1 -p157.7/45
  gmt subplot end
gmt end

# Crop to the ink and read the height of what was actually drawn, in cm
gmt psconvert -A -Te fit3d.ps
height=$(awk '/^%%BoundingBox: / && $5 != "(atend)" {printf "%.2f\n", ($5 - $3) * 2.54 / 72; exit}' fit3d.eps)

rm -f fit3d.ps fit3d.eps	# Measured; there is no baseline for them

# The figure is 10 cm tall and the heading adds about 1 cm above it.  Panels that do not fit come out
# far taller than that: the unfitted cubes of the report reach some 19 cm.
echo "drawn height ${height} cm" > result.txt
awk -v h="$height" 'BEGIN {
	if (h <= 0) { print "nothing was drawn"; exit 1 }
	if (h > 13.0) { printf "the figure is %.2f cm tall, so the 3-D panels do not fit its 10 cm height\n", h; exit 1 }
	print "fits"
}' > verdict.txt

echo "fits" > answer.txt
diff -q --strip-trailing-cr answer.txt verdict.txt
