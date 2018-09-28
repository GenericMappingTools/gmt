#!/bin/bash
gmt begin GMT_colorbar ps
gmt makecpt -T-15/15 -Cpolar > t.cpt
gmt basemap -R0/20/0/1 -JM5i -BWse -Baf 
gmt colorbar -Ct.cpt -Baf -Bx+u"\\232" -By+l@~D@~T -DJBC+e
gmt end
