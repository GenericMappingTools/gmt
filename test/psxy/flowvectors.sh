#!/usr/bin/env bash
ps=flowvectors.ps
gmt makecpt -Cseis -T0/2000 > t.cpt

gmt convert vel.horiz.xyazlen -o0,1,3,2,3 | gmt psxy -Rg -JG-10/30/6i -i0:1,2+s5000,3,4+s5000 -P -Bafg \
  -Ct.cpt -S=0.15i+jc+e+n300/0+p0.25p+h0.5 -W1.5p --PS_COMMENTS=true > $ps
