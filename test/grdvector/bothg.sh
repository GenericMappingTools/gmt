#!/usr/bin/env bash
# Test to deal with issue http://gmt.soest.hawaii.edu/boards/1/topics/6311?r=6315#message-6315
#
# GMT_KNOWN_FAILURE_WINDOWS
#
ps=bothg.ps
gmt pscoast -R-180/180/-70/70 -JM18c -Dc -Sblue -Glightgray -W0.01p,black -Baf -P --FONT_ANNOT_PRIMARY=9p,Helvetica,black -K -Xc > $ps
gmt grdvector -O -K wx_grd000.nc wy_grd000.nc -R-180/180/-70/70 -J -Q4p+e+jc+gwhite -Si50k -W0.75p,white -I10 >> $ps
gmt pscoast -Rg -JG0/0/12c -Dc -Sblue -Glightgray -W0.01p,black -Baf --FONT_ANNOT_PRIMARY=9p,Helvetica,black -O -K -X3c -Y12c >> $ps
gmt grdvector -O wx_grd000.nc wy_grd000.nc -R-180/180/-70/70 -J -Q4p+e+jc+gwhite -Si50k -W0.75p,white -I10 >> $ps
