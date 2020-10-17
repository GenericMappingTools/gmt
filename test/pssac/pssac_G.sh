#!/usr/bin/env bash
#
# Description:

R=9/20/-1.6/1.6
J=X15c/5c
B=1
ps=pssac_G.ps

gmt set PS_MEDIA 42cx40c
gmt pssac seis.sac -R$R -J$J -B$B -BWSen -W0p -K -P -G > $ps
gmt pssac seis.sac -R -J -B$B -BWsen -W0p -K -O -Y6c -G+gblue+z-0.2 >> $ps
gmt pssac seis.sac -R -J -B$B -BWsen -W0p -K -O -Y6c -Gn+gred+z0.2 >> $ps
gmt pssac seis.sac -R -J -B$B -BWsen -W0.1p -K -O -Y6c -Gp+z0.2+t10/13+gblue -Gn+z-0.2+t12/18+gred >> $ps
gmt pssac seis.sac -R -J -B$B -BWsen -W0.1p -K -O -Y6c -Gp+z-0.2+t10/13+gblue -Gn+z0.2+t12/18+gred >> $ps
gmt pssac seis.sac -J -R10.2/15/-1.6/1.6 -B$B -BWSen -W0p -K -O -Y6c -Gn+gred >> $ps

gmt psxy -R -J -K -O -T -X16c -Y-30c >> $ps
gmt pssac seis.sac -R$R -J$J -B$B -BWSen -K -O -G >> $ps
gmt pssac seis.sac -R -J -B$B -BWsen -K -O -Y6c -G+gblue+z-0.2 >> $ps
gmt pssac seis.sac -R -J -B$B -BWsen -K -O -Y6c -Gn+gred+z0.2 >> $ps
gmt pssac seis.sac -R -J -B$B -BWsen -K -O -Y6c -Gp+z0.2+t10/13+gblue -Gn+z-0.2+t12/18+gred >> $ps
gmt pssac seis.sac -R -J -B$B -BWsen -K -O -Y6c -Gp+z-0.2+t10/13+gblue -Gn+z0.2+t12/18+gred >> $ps
gmt pssac seis.sac -J -R10.2/15/-1.6/1.6 -B$B -BWSen -O -Y6c -Gn+gred >> $ps
