#!/bin/bash
#	$Id: trend.sh 14635 2015-07-28 23:06:07Z remko $
# Testing gmt trend1d

ps=trend1.ps

# Created a fake y(x) = (x/50)^2 + (x/60) + 4 + cos(2*pi*x/25) + noise and fit LS model
#gmt math -T10/110/1 T 50 DIV 2 POW 2 MUL T 60 DIV ADD 4 ADD 0 0.25 NRAND ADD T 25 DIV 2 MUL PI MUL COS 2 MUL 2 ADD ADD = data1.txt
gmt trend1d data1.txt -Np2,F1+o0+l25 -Fxm > model.txt
gmt psxy -R0/120/0/20 -JX6i/2.5i -Baf data1.txt -Sc0.1c -Gblack -P -K -Xc > $ps
gmt psxy -R -J -O -K -W1p,blue model.txt >> $ps
# Created a fake y(x) = 4*(x/50)^3 + 3 + noise and fit LS model
#gmt math -T-50/50/1 T 50 DIV 3 POW 4 MUL 3 ADD 0 0.25 NRAND ADD = data2.txt
gmt trend1d data2.txt -NP0,P3 -Fxm > model.txt
gmt psxy -R-50/50/0/6 -J -Baf data2.txt -Sc0.1c -Gblack -O -K -Y3.25i >> $ps
gmt psxy -R -J -O -K -W1p,blue model.txt >> $ps
# Created a fake y(x) = x + 3 + noise and fit LS model using old syntax
#gmt math -T-50/50/1 T 50 DIV 3 ADD 0 0.25 NRAND ADD = data3.txt
gmt trend1d data3.txt -N2 -Fxm > model.txt
gmt psxy -R-50/50/0/6 -J -Baf data3.txt -Sc0.1c -Gblack -O -K -Y3.25i >> $ps
gmt psxy -R -J -O -W1p,blue model.txt >> $ps
gv $ps &
