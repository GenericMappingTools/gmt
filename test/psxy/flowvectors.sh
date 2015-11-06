#!/bin/bash
#	$Id$
ps=flowvectors.ps
gmt makecpt -Cseis -T0/2000/100 -Z > t.cpt

gmt gmtconvert vel.horiz.xyazlen -o0,1,3,2,3 | gmt psxy -Rg -JG-10/30/6i -i0:1,2s5000,3,4s5000 -P -Bafg \
  -Ct.cpt -S=0.15i+jc+e+n300+p0.25p -W1.5p --MAP_VECTOR_SHAPE=0.5 --PS_COMMENTS=true > $ps
