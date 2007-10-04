#!/bin/sh
#	$Id: hexagone.sh,v 1.9 2007-10-04 19:51:34 remko Exp $
#
# Check wrapping around Greenwich

echo -n "$0: Test psxy and -A resampling crossing Greenwich:		"

ps=hexagone.ps

cat > hexagone.dat <<%
-4.5 48.5
-2 43.5
3 42.5
7.5 44
8 49
2.5 51
%

psxy hexagone.dat -R-5/9/42/52 -JM3i -P -L -Gpurple -K > $ps
pscoast -R -J -Dl -Wthin -Ia/thin -N1/thick,red -B2 -O -K --PLOT_DEGREE_FORMAT=D >> $ps

psxy hexagone.dat -R-8.5/-0.5/47/52 -JM3i -Y4i -L -Gpurple -O -K >> $ps
pscoast -R -J -Dl -Wthin -Ia/thin -N1/thick,red -B1 -O -K --PLOT_DEGREE_FORMAT=D >> $ps

psxy hexagone.dat -R1/9/47/52 -JM3i -X4i -L -Gpurple -O -K >> $ps
pscoast -R -J -Dl -Wthin -Ia/thin -N1/thick,red -B1 -O -K --PLOT_DEGREE_FORMAT=D >> $ps

psxy hexagone.dat -R-4/6/42/49.5 -JM3i -Y-4i -L -Gpurple -O -K >> $ps
pscoast -R -J -Dl -Wthin -Ia/thin -N1/thick,red -B1 -O --PLOT_DEGREE_FORMAT=D >> $ps

compare -density 100 -metric PSNR {,orig/}$ps hexagone_diff.png > log 2>&1
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAIL]"
	echo $0 >> ../fail_count.d
else
        echo "[PASS]"
        rm -f fail hexagone_diff.png log
fi
rm -f hexagone.dat .gmtcommands4
