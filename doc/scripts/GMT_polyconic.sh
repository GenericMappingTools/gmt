#!/usr/bin/env bash
gmt coast -R-180/-20/0/90 -JPoly/10c -Bx30g10 -By10g10 -Dc -A1000 -Glightgray -Wthinnest --GMT_THEME=cookbook -ps GMT_polyconic
