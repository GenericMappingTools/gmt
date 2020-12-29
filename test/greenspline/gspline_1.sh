#!/usr/bin/env bash
#

ps=gspline_1.ps

# Figure 2 in Wessel, P. (2009), A general-purpose Green's function-based
#	interpolator, Computers & Geosciences, 35, 1247-1254.


R=-2000/25000/0/11
gmt psbasemap -R-2/25/0/11 -JX6i/3i -P -K -Bx5f1+l"Distance (km)" -By2f1+l"Mg (ppm)" -BWSne -X1.25i -Y2i --FONT_LABEL=18p > $ps
gmt psxy -R$R -J -O -K @Table_4.2.txt -Sc0.075i -Gblack >> $ps
gmt greenspline -R-2000/25000 -I100 @Table_4.2.txt -Sl -Z0 | gmt psxy -R$R -J -O -K -Wthin,. >> $ps
gmt greenspline -R-2000/25000 -I100 @Table_4.2.txt -Sc -Z0 | gmt psxy -R$R -J -O -K -Wthin,- >> $ps
gmt greenspline -R-2000/25000 -I100 @Table_4.2.txt -St0.25 -Z0 | gmt psxy -R$R -J -O -K -Wthin >> $ps
gmt pslegend -R$R -J -O -F+p -Dx5.9i/2.9i+w2.05i+jTR --FONT_ANNOT_PRIMARY=12p << EOF >> $ps
S 0.2i - 0.35i - 0.5p 0.5i Tension (@%6%t@%% = 0.25)
S 0.2i - 0.35i - 0.5p,- 0.5i No tension
S 0.2i - 0.35i - 0.5p,. 0.5i Linear interpolation
EOF
