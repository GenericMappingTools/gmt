#!/usr/bin/env bash
# Originated with issue # 1080 - should be fixed.
ps=quotedlinedash.ps
cat << EOF > line
16.754838373223794 76.81001006098046
15.316102972173212 77.340866955254867
15.292099601867063 77.906339182191076
16.484841857681328 78.524695180781663
18.766828705710051 79.046509399078843
EOF
cat << EOF > p
14.216667 78.066667
14.501667 80.028333
16.563333 76.481667
EOF
gmt set MAP_ANNOT_OBLIQUE 0
gmt pscoast -Wfaint -R17/76.3/16/81+r -Slightblue -JS66/90/12c -P -K -Dh -Xc -Y2c > $ps
gmt psxy p -Fr15.65/78.22 -O -R -J -Wthick,red,- -SqD40k:+gwhite+f4+LD+an+p+u"km" -K >> $ps
gmt psxy p -Sc0.1c -Gblack -O -R -J -Bafg -K >> $ps
gmt psxy line -R -J -O -K -Wthick,blue >> $ps
gmt pscoast -Wfaint -R -Slightblue -J -K -Dh -O -Y13c >> $ps
gmt psxy p -Fr15.65/78.22 -O -R -J -Wthick,red,- -SqD40k:+f4+LD+an+p+u"km" -K >> $ps
gmt psxy p -Sc0.1c -Gblack -O -R -J -Bafg -K >> $ps
gmt psxy line -R -J -O -Wthick,blue >> $ps
