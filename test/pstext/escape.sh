#!/bin/bash
# $Id$
# Check pslib escape functions

ps=escape.ps

gmt pslegend -R0/100/0/100 -JX250p -D0/0+w185p/60p+jLT+l1.2 -C5p/5p -F -K -P -Y2i > $ps << EOF
S 10p - 20p - 1p,255/0/0,-.. 24p @:10:CO@-2@-@:: @:10:injected@::
S 10p - 20p - 1p,255/0/0,-.. 24p @:10:CO@-2@- injected@::
EOF
gmt pstext -R -J -Gred -Wgreen -F+f12p,Helvetica-Bold+jCT -O >> $ps <<< "25 25 M@-w@-=7.0"
