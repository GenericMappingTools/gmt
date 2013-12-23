#!/bin/bash
#	$Id$
# Test grdlandmask across Greenwich

ps=greenwich.ps
echo "0.5	red	1.5	red" > t.cpt
# ORIGINAL:
gmt grdlandmask -R-1.05/0.05/50.95/52.05 -I3s -Glandmask.grd=ns -Df -NNaN/1/1/1/1
gmt grdimage landmask.grd -Jx2.5id -P -Ba1g1 -BWSne -Ct.cpt -K -Xc > $ps
gmt pscoast -R -J -O -K -Df -W0.25p >> $ps
gmt grdlandmask -R-2.05/-0.95/50.95/52.05 -I3s -Glandmask.grd=ns -Df -NNaN/1/1/1/1
gmt grdimage landmask.grd -J -O -Ba1g1 -BWSne -Ct.cpt -K -Y3.25i >> $ps
gmt pscoast -R -J -O -K -Df -W0.25p >> $ps
gmt grdlandmask -R-0.05/1.05/50.95/52.05 -I3s -Glandmask.grd=ns -Df -NNaN/1/1/1/1
gmt grdimage landmask.grd -J -O -K -Ba1g1 -BWSne -Ct.cpt -Y3.25i >> $ps
gmt pscoast -R -J -O -Df -W0.25p >> $ps
rm -f landmask.grd t.cpt

# FAKED TO GIVE CORRECT PS
#grdlandmask -R-1.05/0.05/50.95/52.05 -I3s -Glandmask.grd=ns -Df -NNaN/1/1/1/1
#psbasemap -R -Jx2.5id -P -B+gred -K -Xc > $ps
#grdimage landmask.grd -Jx2.5id -O -Ct.cpt -K >> $ps
#psxy -R -J -O -K -A -Gred << EOF >> $ps
#-1.05	50.95
#-0.95	50.95
#-0.95	52.05
#-1.05	52.05
#EOF
#pscoast -R -J -O -K -Df -W0.25p -Ba1g1 -BWSne >> $ps
#grdlandmask -R-2.05/-0.95/50.95/52.05 -I3s -Glandmask.grd=ns -Df -NNaN/1/1/1/1
#grdimage landmask.grd -J -O -Ba1g1 -BWSne -Ct.cpt -K -Y3.25i >> $ps
#pscoast -R -J -O -K -Df -W0.25p >> $ps
#grdlandmask -R-0.05/1.05/50.95/52.05 -I3s -Glandmask.grd=ns -Df -NNaN/1/1/1/1
#grdimage landmask.grd -J -O -K -Ct.cpt -Y3.25i >> $ps
#psxy -R -J -O -K -A -Gred << EOF >> $ps
#-0.05	50.95
#0.05	50.95
#0.05	52.05
#-0.05	52.05
#EOF
#pscoast -R -J -O -Df -W0.25p -Ba1g1 -BWSne >> $ps

