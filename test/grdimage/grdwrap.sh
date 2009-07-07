#!/bin/sh
#
#	$Id: grdwrap.sh,v 1.5 2009-07-07 00:59:18 remko Exp $

. ../functions.sh
header "Test grdimage for wrapping of global grid"

ps=grdwrap.ps
grdmath -Rg -I1 X SIND Y COSD MUL = t.grd=ns/0.0001
makecpt -Cpolar -T-1/1/0.01 > t.cpt

grdimage t.grd -Ct.cpt -JQ-147/6i -Ct.cpt -B30 -K > $ps
grdimage t.grd -Ct.cpt -JQ-147.533/6i -Ct.cpt -B30 -O -Y3.75i >> $ps

rm -f t.grd t.cpt .gmtcommands4

pscmp
