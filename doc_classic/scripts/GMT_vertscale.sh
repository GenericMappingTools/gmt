#!/usr/bin/env bash
#
ps=GMT_vertscale.ps
gmt math -T-8/6/0.01 -N3/0 -C2 T 3 DIV 2 POW NEG EXP T PI 2 MUL MUL COS MUL 50 MUL = t.txt
gmt pswiggle -R-10/10/-3/3 -JM6i -Baf -Z100i -DjRM+w100+lnT t.txt -Tfaint -W1p -BWSne -P --MAP_FRAME_TYPE=plain > $ps
