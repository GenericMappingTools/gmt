#!/bin/bash
#	$Id: quotedclip.sh,v 1.1 2011-03-21 21:49:59 guru Exp $
#
# Check clip path and delayed text using -Sq:+e

. ../functions.sh
header "Test psxy for quoted lines with clipping and delayed text"

ps=quotedclip.ps

psbasemap -R0/10/0/10 -JX15c/10c -Ggray70 -K -P > $ps

psxy -R -J -W1p,black -Sqn1:+Lh+e+s18 -O -K << EOF >> $ps
> "The quick brown fox jumps over the lazy dog"
0 8
10 5
EOF

# small polygon (is not clipped by psxy text)
psxy -R -J -L -Gorange -W0.5p -O -K << EOF >> $ps
7 4
7 8
9 8
9 4
EOF

psclip -Ct -O -K >> $ps

# the box should not be aligned with the text baseline:
psxy -R -J -W1p,red -Sqn1:+Lh+glightorange+c0+s12 -O << EOF >> $ps
> "The box should not be aligned with the text baseline"
0 1
10 3
EOF

pscmp
