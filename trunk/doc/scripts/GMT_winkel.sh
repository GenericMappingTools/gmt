#!/bin/bash
#	$Id: GMT_winkel.sh,v 1.8 2011-06-10 23:29:27 guru Exp $
#
. ./functions.sh

pscoast -Rd -JR4.5i -Bg30/g15 -Dc -A10000 -Gburlywood4 -Swheat1 -P > GMT_winkel.ps
