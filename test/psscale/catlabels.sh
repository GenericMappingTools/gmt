#!/usr/bin/env bash
# Testing the auto-labeling of categorical CPTs.
ps=catlabels.ps
gmt set FONT_ANNOT_PRIMARY 9p,Helvetica,black
gmt makecpt -T0/3/1 -Ccubhelix -F+c0- > t.cpt
gmt psxy -R0/20/0/30 -Jx1c -P -K -Y0 -T > $ps
gmt psscale -O -K -Ct.cpt -Dx8c/22c+w12c/0.5c+jTC+h -Y2.5c >> $ps
gmt makecpt -T0/3/1 -Ccubhelix -F+c > t.cpt
gmt psscale -O -K -Ct.cpt -Dx8c/19c+w12c/0.5c+jTC+h -Bxaf >> $ps
gmt psscale -O -K -Ct.cpt -Dx8c/16c+w12c/0.5c+jTC+h -Li0.4c >> $ps
gmt makecpt -T0/3/1 -Ccubhelix -F+cNight,Trees,Sediment,Water > t.cpt
gmt psscale -O -K -Ct.cpt -Dx8c/13c+w12c/0.5c+jTC+h -Li0.4c >> $ps
gmt makecpt -T0/3/1 -Ccubhelix -F+cA > t.cpt
gmt psscale -O -K -Ct.cpt -Dx8c/10c+w12c/0.5c+jTC+h >> $ps
gmt makecpt -T0/3/1 -Ccubhelix -F+c1 > t.cpt
gmt psscale -O -K -Ct.cpt -Dx8c/7c+w12c/0.5c+jTC+h >> $ps
gmt makecpt -T0/3/1 -Ccubhelix -F+c9- > t.cpt
gmt psscale -O -K -Ct.cpt -Dx8c/4c+w12c/0.5c+jTC+h >> $ps
gmt makecpt -T0/3/1 -Ccubhelix -F+cG- > t.cpt
gmt psscale -O -Ct.cpt -Dx8c/1c+w12c/0.5c+jTC+h -Li0.4c >> $ps
