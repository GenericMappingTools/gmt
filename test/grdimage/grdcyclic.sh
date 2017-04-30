#!/bin/bash
#
#	$Id$

ps=grdcyclic.ps

gmt makecpt -Cjet -T0/1000 -Ww > cyclic.cpt
gmt makecpt -Cjet -T-5000/3000 > t.cpt

gmt grdimage afr.grd -Ccyclic.cpt -JM4.5i -P -K -Baf -BWSNe -X1.5i -Y0.75i > $ps
gmt psscale -DJRM+mc -Ccyclic.cpt -Baf+l"CYCLIC" -Rafr.grd -J -O -K >> $ps
gmt grdimage afr.grd -Ct.cpt -JM4.5i -O -K -Baf -BWsNe -Y4.9i >> $ps
gmt psscale -DJRM+mc -Ct.cpt -Baf+l"ABSOLUTE" -Rafr.grd -J -O >> $ps
