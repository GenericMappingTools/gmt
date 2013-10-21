#!/bin/bash
# Test the gmt gmt2kml application


gmt pscoast -R-15/2/50/59:30 -Jm1i -M -W0.25p -Di > coast.txt
gmt gmt2kml coast.txt -Fl -W1p,red > coast.kml
# reduce exponent digits (e-015 -> e-15):
sed -i -e 's/e-0\([0-9][0-9]\)/e-\1/g' coast.kml
diff --strip-trailing-cr coast.kml "${src:-.}"/coast.kml > fail

# test -C option
makecpt -Ccategorical > categories.cpt

gmt gmt2kml "${src:-.}"/pointsets.txt -Ccategories.cpt > pointsets.kml
#todo: diff --strip-trailing-cr pointsets.kml "${src:-.}"/pointsets.kml > fail

gmt gmt2kml "${src:-.}"/lines.txt -Fl -Ccategories.cpt > lines.kml
#todo: diff --strip-trailing-cr lines.kml "${src:-.}"/lines.kml > fail
