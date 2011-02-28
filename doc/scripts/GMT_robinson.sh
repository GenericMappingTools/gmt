#!/bin/bash
#	$Id: GMT_robinson.sh,v 1.5 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

pscoast -Rd -JN4.5i -Bg30/g15 -Dc -A10000 -Ggray -P > GMT_robinson.ps
