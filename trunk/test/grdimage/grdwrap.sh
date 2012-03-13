#!/bin/bash
#
#	$Id$

. ./functions.sh
header "Test grdimage for wrapping of global grid"

grdmath -Rg -I1 X SIND Y COSD MUL = t.nc=ns/0.0001
makecpt -Cpolar -T-1/1/0.01 > t.cpt

grdimage t.nc -Ct.cpt -JQ-147/6i -Ct.cpt -B30 -K > $ps
grdimage t.nc -Ct.cpt -JQ-147.533/6i -Ct.cpt -B30 -O -Y3.75i >> $ps

pscmp
