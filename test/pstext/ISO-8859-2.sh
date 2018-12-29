#!/bin/bash
#
# script for ISO-8859-2 character test
#
ps=ISO-8859-2.ps
#
gmt set PS_CHAR_ENCODING ISO-8859-2

gmt pscoast -R14.633/14.892/44.67/44.94 -JM6i -Df -Wfaint,gray -Baf -BWSne+t"Test_ISO-8859-2" -Xc -P -K > $ps
gmt pstext ISO-8859-2.txt -R -J -F+f12p -O >> $ps
