#!/usr/bin/env bash
# Further exploration of both soft and hard hinges

ps=hardsoft.ps

gmt makecpt -Cgeo -T-1/2 > t.cpt
gmt psscale -Dx0/0+w6i+jBL -Ct.cpt -Bxaf -By+l"a)" -K -P -X0.75i > $ps
gmt makecpt -Cgeo+h0.5 -T-1/2 > t.cpt
gmt psscale -Dx0.75i/0+w6i+jBL -Ct.cpt -Bxaf -By+l"b)" -O -K >> $ps
gmt makecpt -Cgeo+h-0.5 -T-1/2 > t.cpt
gmt psscale -Dx1.5i/0+w6i+jBL -Ct.cpt -Bxaf -By+l"c)" -O -K >> $ps
gmt makecpt -Cgeo -T1/2 > t.cpt
gmt psscale -Dx2.25i/0+w6i+jBL -Ct.cpt -Bxaf -By+l"d)" -O -K >> $ps
gmt makecpt -Cgeo -T-2/-1 > t.cpt
gmt psscale -Dx3i/0+w6i+jBL -Ct.cpt -Bxaf -By+l"e)" -O -K >> $ps

gmt makecpt -Cpolar -T-10/20 > t.cpt
gmt psscale -Dx3.75i/0+w6i+jBL -Ct.cpt -Bxaf -By+l"f)" -O -K >> $ps
gmt makecpt -Cpolar+h -T-10/20 > t.cpt
gmt psscale -Dx4.5i/0+w6i+jBL -Ct.cpt -Bxaf -By+l"g)" -O -K >> $ps
gmt makecpt -Cpolar+h-5 -T-10/20 > t.cpt
gmt psscale -Dx5.26i/0+w6i+jBL -Ct.cpt -Bxaf -By+l"h)" -O -K >> $ps
gmt makecpt -Cpolar+h -T0/20 > t.cpt
gmt psscale -Dx6i/0+w6i+jBL -Ct.cpt -Bxaf -By+l"i)" -O -K >> $ps
gmt makecpt -Cpolar+h-5 -T-10/-5 > t.cpt
gmt psscale -Dx6.75i/0+w6i+jBL -Ct.cpt -Bxaf -By+l"j)" -O -K >> $ps
gmt pstext -R0/6/-2/9 -Jx1i/-0.2i -Y6.5i -F+f10p+jBL -O -N<< EOF >> $ps
0 0 a) -Cgeo -T-1/2 [Normal hard hinge behavior at z = 0]
0 1 b) -Cgeo+h0.5 -T-1/2 [Relocate hard hinge to +0.5 km]
0 2 c) -Cgeo+h-0.5 -T-1/2 [Relocate hard hinge to -0.5 km]
0 3 d) -Cgeo -T1/2 [Range above hard hinge z = 0; lower CPT removed, no hinge]
0 4 e) -Cgeo -T-2/-1 [Range below hard hinge z = 0; upper CPT removed, no hinge]
0 5 f) -Cpolar -T-10/20 [Normal behavior is to ignore soft hinges]
0 6 g) -Cpolar+h -T-10/20 [Activate hinge at z = 0]
0 7 h) -Cpolar+h-5 -T-10/20 [Activate hinge at z = -5]
0 8 i) -Cpolar+h -T0/20 [Activate hinge z = 0 but range above; lower CPT removed, no hinge]
0 9 j) -Cpolar+h-5 -T-10/-5 [Activate hinge at z = -5 but range is below; upper CPT removed, no hinge]
EOF
