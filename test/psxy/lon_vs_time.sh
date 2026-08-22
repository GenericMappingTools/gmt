#!/usr/bin/env bash
# Test longitude vs time and time versus longitude.
ps=lon_vs_time.ps
gmt math -T1994/2015/1 T 1993 SUB 30 MUL 100 SUB 360 FMOD = | awk '{printf "%s-01-01T\t%s\n", $1, $2 }' > tx.txt
gmt math -T1994/2015/1 T 1993 SUB 30 MUL 100 SUB 360 FMOD = | awk '{printf "%s\t%s-01-01T\n", $2, $1 }' > xt.txt
gmt psxy -R1993-01-01T/2016-06-01T/-41/319 -JX15ct/10cd -f0T,1x -Bx5Y -By30 -P -K -BWSne -Sc0.1i -Ggreen -Wthin tx.txt -Xc > $ps
gmt psxy -R-41/319/1993-01-01T/2016-06-01T -JX15cd/10ct -f0x,1T -By5Y -Bx30 -O -BWSne -Sc0.1i -Ggreen -Wthin xt.txt -Y13c >> $ps
