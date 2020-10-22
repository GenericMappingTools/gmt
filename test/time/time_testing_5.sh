#!/usr/bin/env bash
#
#
# This script runs some simple test to verify the that new time scheme
# has been implemented successfully

# Test 5:
# We plot a data set with gmt psxy in two ways:  The bottom plot is based on
# calendar data whereas the upper plot is based on time relative to the
# users epoch.  We use the same data as before to generate the absolute
# coordinates using the Apollo 11 epoch.

ps=time_testing_5.ps

gmt sample1d -I0.5 << EOF > tt5.d
0	0
4	1
EOF
gmt convert tt5.d -fi0t -fo0T --TIME_EPOCH=1969-07-21T02:56:00 --TIME_UNIT=d \
  | gmt psxy -R1969-07-18T/1969-07-28T/-0.1/1.1 -JX9T/6 -Bpxa7Rf1d -Bpy0.2 -Bsxa1O -Sc0.15 -Gred --FORMAT_DATE_MAP="-o yyyy" --FONT_ANNOT_PRIMARY=9p -K > $ps
gmt psxy tt5.d -R -JX9t/6 --TIME_EPOCH=1969-07-21T02:56:00 -S+0.25 --TIME_UNIT=d --FORMAT_DATE_MAP="-o yyyy" --FONT_ANNOT_PRIMARY=9p -O >> $ps
