#!/bin/bash
#	$Id$
#
. ./functions.sh

header "Test horizontal/vertical color bar with/without flipping"

gmtset FONT_ANNOT_PRIMARY 10p FONT_LABEL 14p PROJ_LENGTH_UNIT cm

makecpt -T-6/6/1 -Cseis -D > tmp.cpt

plot () {
$psscale -D00/04/8/0.5 -K $1
$psscale -D04/04/8/0.5 -Aa -O -K
$psscale -D07/04/8/0.5 -Al -O -K
$psscale -D11/04/8/0.5 -A -O -K
$psscale -D18/00/8/0.5h -A -O -K
$psscale -D18/03/8/0.5h -Al -O -K
$psscale -D18/05/8/0.5h -Aa -O -K
$psscale -D18/08/8/0.5h -O $2
}

psscale="psscale -E -Ctmp.cpt -B:Range:/:m:"
plot -Y2 -K > $ps
psscale="psscale -E -Ctmp.cpt -Ac -B1:Range:/:m:"
plot "-Y9 -O" >> $ps

pscmp
