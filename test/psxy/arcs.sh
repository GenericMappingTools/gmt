#!/usr/bin/env bash
#
# Plot a few ellipses and wedges with different fills and outlines
# changed via segment headers

ps=arcs.ps

gmt psxy -R0/4/0/4 -Jx1i -P -W5p -S1i -X2i -Y2i << EOF > $ps
1 1 1 c
1 3.2 0 1.6 0.8 e
> -W3p,blue -Ggreen
1 2 -30 210 1 w
> -Gp7+fgreen+r100 -W5p,red
1 3 30 90 1 w
> -Gred -W5p
3 1 -45 2 1 e
> -Gp12+fred+byellow+r100 -W-
3 3 45 2 1 e
EOF

