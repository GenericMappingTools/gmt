#!/bin/bash
#	$Id: GMT_hammer.sh,v 1.8 2011-06-10 23:29:27 guru Exp $
#
. ./functions.sh

pscoast -Rg -JH4.5i -Bg30/g15 -Dc -A10000 -Gblack -Scornsilk -P > GMT_hammer.ps
