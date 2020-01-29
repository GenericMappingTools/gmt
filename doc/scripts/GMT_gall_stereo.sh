#!/usr/bin/env bash
gmt set FORMAT_GEO_MAP dddA
gmt coast -R-180/180/-60/80 -JCyl_stere/0/45/12c -Bxa60f30g30 -Bya30g30 -Dc -A5000 -Wblack -Gseashell4 -Santiquewhite1 -ps GMT_gall_stereo
