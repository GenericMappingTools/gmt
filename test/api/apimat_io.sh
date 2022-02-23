#!/usr/bin/env bash
#
# Test the C API for reading and writing native matrices.
# We read @hotspots.txt and write it out again, then plot it.

ps=apimat_io.ps
testapi_matrix_io
gmt pscoast -Rg -JQ0/18c -Glightgreen -Baf -BWSne+t"Testing Matrix i/o" -K -P > $ps
gmt psxy -R -J -O -K test_hotspots.txt -Sc0.2c -Gred >> $ps
gmt convert test_hotspots.txt -o0,1,t | gmt pstext -R -J -O -F+f6p+jBL -Dj4p >> $ps
