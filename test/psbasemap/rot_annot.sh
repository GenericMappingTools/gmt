#!/bin/bash
#	Test Cartesian annotation for different view angles
ps=rot_annot.ps
gmt psbasemap -R680.095/712.095/7105.528/7140.578 -Jx1:300 -p45/35 -BWSNE -Bpx10+u"@:8:000m@::" -Bpy10+u"@:6:000m@::"  -P -K > $ps
gmt psbasemap -R680.095/712.095/7105.528/7140.578 -Jx1:300 -p225/35 -BWSNE -Bpx10+u"@:8:000m@::" -Bpy10+u"@:6:000m@::" -P -O -Y5i >> $ps
