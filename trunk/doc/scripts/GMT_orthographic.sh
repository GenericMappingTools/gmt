#!/bin/bash
#	$Id: GMT_orthographic.sh,v 1.10 2011-06-24 21:13:45 guru Exp $
#
. ./functions.sh

pscoast -Rg -JG-75/41/4.5i -Bg -Dc -A5000 -Gpink -Sthistle -P > GMT_orthographic.ps
