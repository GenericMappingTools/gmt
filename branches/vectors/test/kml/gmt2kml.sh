#!/bin/bash
# Test the gmt2kml application

. ../functions.sh

header "Test gmt2kml by plotting coastline of UK"
pscoast -R-15/2/50/59:30 -Jm1i -M -W0.25p -Di > coast.txt
gmt2kml coast.txt -Fl -W1p,red > coast.kml
diff --strip-trailing-cr coast.kml orig/coast.kml > fail

passfail gmt2kml
rm -f coast.txt coast.kml
