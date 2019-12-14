#!/usr/bin/env bash
#
# Check if psmeca works with various columns

ps=meca_input_columns.ps

gmt psmeca -R239/240/34/35 -JX4c -Ba0 -Sa1c -K > $ps << EOF
239.384 34.556 12.   180     18   -88  5
EOF

gmt psmeca -R -J -Ba0 -Sa1c -K -O -X5c >> $ps << EOF
239.384 34.556 12.   180     18   -88  5 0 0
EOF

gmt psmeca -R -J -Ba0 -Sa1c -K -O -X5c >> $ps << EOF
239.384 34.556 12.   180     18   -88  5 Title
EOF

gmt psmeca -R -J -Ba0 -Sa1c -K -O -X5c >> $ps << EOF
239.384 34.556 12.   180     18   -88  5 Long Title
EOF

gmt psmeca -R -J -Ba0 -Sa1c -K -O -X5c >> $ps << EOF
239.384 34.556 12.   180     18   -88  5 0 0 Long Title
EOF

gmt psmeca -R -J -Ba0 -Sa1c -K -O -X-20c -Y5c -C1p >> $ps << EOF
239.384 34.556 12.   180     18   -88  5
EOF

gmt psmeca -R -J -Ba0 -Sa1c -K -O -X5c -C1p >> $ps << EOF
239.384 34.556 12.   180     18   -88  5 0 0
EOF

gmt psmeca -R -J -Ba0 -Sa1c -K -O -X5c -C1p >> $ps << EOF
239.384 34.556 12.   180     18   -88  5 Title
EOF

gmt psmeca -R -J -Ba0 -Sa1c -K -O -X5c -C1p >> $ps << EOF
239.384 34.556 12.   180     18   -88  5 Long Title
EOF

gmt psmeca -R -J -Ba0 -Sa1c -K -O -X5c -C1p >> $ps << EOF
239.384 34.556 12.   180     18   -88  5 0 0 Long Title
EOF

gmt psmeca -R -J -Ba0 -Sa1c -K -O -X-20c -Y5c -C1p >> $ps << EOF
239.384 34.556 12.   180     18   -88  5 239.8 34.7
EOF

gmt psmeca -R -J -Ba0 -Sa1c -K -O -X5c -C1p >> $ps << EOF
239.384 34.556 12.   180     18   -88  5 239.8 34.7 Title
EOF

gmt psmeca -R -J -Ba0 -Sa1c -K -O -X5c -C1p >> $ps << EOF
239.384 34.556 12.   180     18   -88  5 239.8 34.7 Long Title
EOF

gmt psmeca -R -J -Ba0 -Sm1c -K -O -X5c -C1p >> $ps << EOF
239.384 34.556 12. 7.68 0.09 -7.77 1.39 4.52 -3.26 26 X Y 010176A
EOF

gmt psxy -R -J -T -O >> $ps
