#!/bin/bash
#       $Id$
#
R=-R20/-1/71/-35r
gmt pscoast $R -JE-100/40/6.5i -B10g10 -Dc -A10000 -Glightgray -Wthinnest -P -Xc > pscoast_Madagascar.ps
