#!/usr/bin/env bash
gmt begin GMT_tut_10
	gmt text -R0/7/0/5 -Jx1i -B -F+f30p,Times-Roman,DarkOrange+jBL << EOF
1  1  It's P@al, not Pal!
1  2  Try @%33%ZapfChancery@%% today
1  3  @~D@~g@-b@- = 2@~pr@~G@~D@~h.
1  4  University of Hawaii at M@!a\225noa
EOF
gmt end show
