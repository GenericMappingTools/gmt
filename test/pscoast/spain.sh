#!/bin/bash
#
# Make sure bug #1156 does not reappear.

ps=spain.ps

gmt pscoast -R-10/0/36/45 -JM14c -Ba -W0.25p -EES,+gred -Ggreen -P -Xc > $ps
