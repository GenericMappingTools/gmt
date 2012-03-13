#!/bin/bash
#
#	$Id$

. ./functions.sh
header "Test orthogonal annotations in psbasemap"

psbasemap="psbasemap -R0/1000/0/1000 -JX2i/2i -B200f100WESN -B:horizontal:/:vertical: --FONT_ANNOT_PRIMARY=10p --FONT_LABEL=16p"
$psbasemap --MAP_ANNOT_ORTHO=we -Xf1i -Yf1i -P -K > $ps
$psbasemap --MAP_ANNOT_ORTHO=sn -Xf4.5i -O -K >> $ps
$psbasemap --MAP_ANNOT_ORTHO=wesn -Xf1i -Yf4.5i -O -K >> $ps
$psbasemap --MAP_ANNOT_ORTHO= -Xf4.5i -O >> $ps

pscmp
