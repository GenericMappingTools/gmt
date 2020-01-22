#!/usr/bin/env bash
# Testing vertical psscale panel sizing if -+e +n and reverse scale.
ps=versclpanel.ps
gmt set FONT_ANNOT_PRIMARY 9p,Helvetica,black
gmt makecpt -T-200/1000 -Crainbow > t.cpt
gmt psxy -R0/25/0/30 -Jx1c -P -K -X0 << EOF > $ps
0	12
25	12
EOF
gmt psscale -O -K -Ct.cpt -Dx0c/12c+w16c/0.5c+jLM+e+n -Bxaf -By+lkm -F+gwhite+p+i+s -X1.5c >> $ps 
gmt psscale -O -K -Ct.cpt -Dx2.5c/12c+w-16c/0.5c+jLM+e+n -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
gmt psscale -O -K -Ct.cpt -Dx5c/12c+w-16c/0.5c+jLM+ebf -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
gmt psscale -O -K -Ct.cpt -Dx7.5c/12c+w16c/0.5c+jLM+ebf -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
gmt psscale -O -K -Ct.cpt -Dx10c/12c+w-16c/0.5c+jLM+eb+n -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
gmt psscale -O -K -Ct.cpt -Dx12.5c/12c+w-16c/0.5c+jLM+ef+n -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
gmt psscale -O -K -Ct.cpt -Dx15c/12c+w16c/0.5c+jLM+eb+n -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
gmt psscale -O -Ct.cpt -Dx17.5c/12c+w16c/0.5c+jLM+ef+n -Bxaf -By+lkm -F+gwhite+p+i+s >> $ps
