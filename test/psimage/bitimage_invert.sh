#!/usr/bin/env bash
# Test -I option for inverting 2-color (1-bit) images
# This tests the fix for issue #8795

ps=bitimage_invert.ps

# Normal image (black Vader on white background)
gmt psimage @vader1.png -P -Dx0/0+w2i -F+pfaint -K > $ps

# Inverted image (white Vader on black background) using -I
gmt psimage @vader1.png -I -Dx2.5i/0+w2i -F+pfaint -O -K >> $ps

# Inverted with color change: red background, yellow foreground
gmt psimage @vader1.png -I -Gred+b -Gyellow+f -Dx5i/0+w2i -F+pfaint -O >> $ps
