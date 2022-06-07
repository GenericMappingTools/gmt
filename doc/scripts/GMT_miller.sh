#!/usr/bin/env bash
gmt coast -R-90/270/-80/90 -Jj1:400000000 -Bx45g45 -By30g30 -Dc -A10000 -Gkhaki -Wthinnest -Sazure --GMT_THEME=cookbook -view GMT_miller
