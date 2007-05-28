#!/bin/sh
#
#	$Id: position1.sh,v 1.6 2007-05-28 22:21:03 pwessel Exp $

ps=position1.ps
font=0	# Helvetica
#font=4	# Times-Roman
#font=8	# Courier
#font=15	# AvantGardeDemi
echo -n "$0: Test psbasemap's annotation alignment:		"

psbasemap="psbasemap --ANNOT_FONT_SIZE=24p --ANNOT_FONT=$font --BASEMAP_AXES=WESN"

$psbasemap -JX8i/5i -R0/12/0/12 -B1g1/1g1SW -K > $ps
pstext -R -J -O -K >> $ps <<%
 1 1 24 0 $font BR 1
 1 2 24 0 $font BC 1
 1 3 24 0 $font BL 1
 2 1 24 0 $font BR 2
 2 2 24 0 $font BC 2
 2 3 24 0 $font BL 2
 3 1 24 0 $font BR 3
 3 2 24 0 $font BC 3
 3 3 24 0 $font BL 3
 4 1 24 0 $font BR BR
 4 2 24 0 $font BC BC
 4 3 24 0 $font BL BL
 6 1 24 0 $font BR 1A
 6 2 24 0 $font BC 1A
 6 3 24 0 $font BL 1A
 7 1 24 0 $font BR 2A
 7 2 24 0 $font BC 2A
 7 3 24 0 $font BL 2A
 8 1 24 0 $font BR Z1
 8 2 24 0 $font BC Z1
 8 3 24 0 $font BL Z1
 9 1 24 0 $font BR 9
 9 2 24 0 $font BC 9
 9 3 24 0 $font BL 9
10 1 24 0 $font BR 10
10 2 24 0 $font BC 10
10 3 24 0 $font BL 10
11 1 24 0 $font BR 11
11 2 24 0 $font BC 11
11 3 24 0 $font BL 11
 1 5 24 0 $font BL 01111111111
 1 6 24 0 $font BL 11111111110
 1 7 24 0 $font BL 01234567890
6.5 5 24 0 $font BC 01111111111
6.5 6 24 0 $font BC 11111111110
6.5 7 24 0 $font BC 01234567890
12 5 24 0 $font BR 01111111111
12 6 24 0 $font BR 11111111110
12 7 24 0 $font BR 01234567890
%
pstext -Wgreen -R -J -O -K >> $ps <<%
 1 9 24 0 $font BR 10@+-1@+
 5 9 24 0 $font MR 10@+-1@+
 9 9 24 0 $font TR 10@+-1@+
 2 9 24 0 $font BR oo
 6 9 24 0 $font MR oo
10 9 24 0 $font TR oo
%
$psbasemap -J -R0/1.2/0/1.2 -B0.1/0.1NE -O >> $ps

rm -f .gmtcommands4

compare -density 100 -metric PSNR position1_orig.ps position1.ps position1_diff.png > log
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAILED]"
else
        echo "[OK"]
        rm -f fail position1_diff.png log
fi
