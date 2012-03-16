#!/bin/bash
#	$Id$
#
# Plot all the symbols on a 1x1 inch grid pattern

header "Test psxy with various circles, ellipses and wedges"

psxy -R0/4/0/4 -Jx1i -P -W5p -S1i -X2i -Y2i << EOF > $ps
1 1 1 c
1 3.2 0 1.6 0.8 e
> -W3p,blue -Ggreen
1 2 -30 210 1 w
> -Gp100/7:Fgreen -W5p,red
1 3 30 90 1 w
> -Gred -W5p
3 1 -45 2 1 e
> -Gp100/12:FredByellow -W-
3 3 45 2 1 e
EOF

pscmp
