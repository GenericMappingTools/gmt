#!/usr/bin/env bash
# Testing horizontal psscale panel sizing if -+e +n and reverse scale.
ps=horsclpanel.ps
gmt set FONT_ANNOT_PRIMARY 9p,Helvetica,black
gmt makecpt -T-200/1000 -Crainbow > t.cpt
gmt psxy -R0/20/0/30 -Jx1c -P -K -Y0 << EOF > $ps
8	0
8	30
EOF
gmt psscale -O -K -Ct.cpt -Dx8c/22c+w12c/0.5c+jTC+e+h+n -Bxaf -By+lkm -F+gwhite+p+i+s -Y2.5c >> $ps
gmt psscale -O -K -Ct.cpt -Dx8c/19c+w-12c/0.5c+jTC+e+h+n -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
gmt psscale -O -K -Ct.cpt -Dx8c/16c+w-12c/0.5c+jTC+ebf+h -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
gmt psscale -O -K -Ct.cpt -Dx8c/13c+w12c/0.5c+jTC+ebf+h -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
gmt psscale -O -K -Ct.cpt -Dx8c/10c+w-12c/0.5c+jTC+eb+h+n -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
gmt psscale -O -K -Ct.cpt -Dx8c/7c+w-12c/0.5c+jTC+ef+h+n -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
gmt psscale -O -K -Ct.cpt -Dx8c/4c+w12c/0.5c+jTC+eb+h+n -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
gmt psscale -O -Ct.cpt -Dx8c/1c+w12c/0.5c+jTC+ef+h+n -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
