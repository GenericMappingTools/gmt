#!/bin/bash
# Testing psscale -F option etc.
ps=horizontal.ps
gmt set FONT_ANNOT_PRIMARY 12p FONT_LABEL 16p
gmt makecpt -T-200/1000/100 -Crainbow > t.cpt
gmt psscale -Ct.cpt -Dx3i/0i/6i/0.2ih -Bxaf -P -F+p1p+glightgray -Xc -K > $ps
gmt psscale -Ct.cpt -Dx3i/0i/6i/0.2ih -Bxaf+l"topography" -By+lkm -F+p1p+i -O -K -Y1.1i >> $ps
gmt psscale -Ct.cpt -Dx3i/0i/6i/0.2ih -Bxaf+l"topography" -By+lkm -F+p1p -O -K -Y1.2i >> $ps
gmt psscale -Ct.cpt -Dx3i/0i/6i/0.2ih -Bxaf+l"topography" -By+lkm -F+p1p -A -O -K -Y0.5i >> $ps
gmt psscale -Ct.cpt -Dx3i/0i/6i/0.2ih -Bxaf+l"topography" -By+lkm -F+p1p -Aa -O -K -Y1.4i >> $ps
gmt psscale -Ct.cpt -Dx3i/0i/6i/0.2ih -Bxaf+l"topography" -By+lkm -F+p1p -Al -O -K -Y1.0i >> $ps
gmt makecpt -T-20/10/1 -Crainbow > t.cpt
gmt psscale -Ct.cpt -Dx3i/0i/6i/0.2ih -Bxaf+l"topography" -By+lkm -F+p1p+r -Al -O -K -Y1.2i >> $ps
gmt set FONT_ANNOT_PRIMARY 16p FONT_LABEL 24p
gmt psscale -Ct.cpt -Dx3i/0i/6i/0.2ih -Bxaf+l"topography" -By+lmiles -F+p1p+r -Al -O -K -Y1.2i >> $ps
gmt psscale -Ct.cpt -Dx3i/0i/6i/0.2ih -Bxaf+l"topography" -F+p1p+glightgray+s -Al -O -Y1.2i >> $ps
