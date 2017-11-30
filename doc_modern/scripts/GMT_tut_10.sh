#!/bin/bash
#	$Id$
#
gmt pstext -R0/7/0/5 -Jx1i -P -Ba -F+f30p,Times-Roman,DarkOrange+jBL << EOF > GMT_tut_10.ps
1  1  It's P@al, not Pal!
1  2  Try @%33%ZapfChancery@%% today
1  3  @~D@~g@-b@- = 2@~pr@~G@~D@~h.
1  4  University of Hawaii at M@!a\225noa
EOF
