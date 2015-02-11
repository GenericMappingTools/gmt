#!/bin/bash
# More testing of issue #667
ps=gridlines.ps
lon0=188.144925731
lat0=80.7167845195
gmt psbasemap -Rg -JG$lon0/$lat0/1900/0/0/0/170/180/3i -Bxa30f10g10 -Bya5f5g5 -BWSne -P -K > $ps
lon0=210.651901558 
lat0=78.8472134393 
gmt psbasemap -Rg -JG$lon0/$lat0/1900/0/0/0/170/180/3i -Bxa30f10g10 -Bya5f5g5 -BWSne -O -K -X3.5i >> $ps
lon0=190.718850211
lat0=82.2203558311
gmt psbasemap -Rg -JG$lon0/$lat0/1900/0/0/0/170/180/3i -Bxa30f10g10 -Bya5f5g5 -BWSne -O -K -X-3.5i -Y4.5i >> $ps
lon0=280.969616634
lat0=58.7193421224
gmt psbasemap -Rg -JG$lon0/$lat0/2513/0/0/0/200/180/3i -Bxa30f10g10 -Bya5f5g5 -BWSne -O -X3.5i >> $ps
