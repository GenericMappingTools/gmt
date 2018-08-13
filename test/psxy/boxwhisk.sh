#!/bin/bash
#
# Check that vertical and horizontal box-and-whisker symbols work
ps=boxwhisk.ps

gmt psxy -R0/10/0/10 -JX4i -P -Bafg5 -Xc -EY+p1p+w10p -Sp -Glightblue -W0.5p -K << EOF > $ps
5 5 2 3 6 9
EOF
gmt psxy -R -J -Bafg5 -Y5i -EX+p1p+w10p -Sp -Glightred -W0.5p -O << EOF >> $ps
5 5 2 3 6 9
EOF
