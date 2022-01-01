#!/usr/bin/env bash
#
# Check unusual beachballs that used to fail
#
ps=meca_unusual_beachball.ps

gmt psmeca -Sm2c+m -JX12c -R0/5/0/5 -Gred -T0 -W0.5p > $ps << EOF
1 1 10 -0.0000 -2.2350 -0.5587 -1.2374 -0.3892 -0.5304 24 isuse #2397
2 1 15 -0.00 1.08 -1.08 -0.75 -0.07 -0.46 24		      issue #4225
EOF
