#!/bin/bash
#	$Id: GMT_robinson.sh,v 1.6 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

pscoast -Rd -JN4.5i -Bg30/g15 -Dc -A10000 -Ggray -P > GMT_robinson.ps
