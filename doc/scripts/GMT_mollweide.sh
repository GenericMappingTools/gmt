#!/bin/bash
#	$Id: GMT_mollweide.sh,v 1.8 2011-06-10 23:29:27 guru Exp $
#
. ./functions.sh

pscoast -Rd -JW4.5i -Bg30/g15 -Dc -A10000 -Gtomato1 -Sskyblue -P > GMT_mollweide.ps
