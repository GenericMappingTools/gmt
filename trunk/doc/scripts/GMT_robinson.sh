#!/bin/bash
#	$Id: GMT_robinson.sh,v 1.8 2011-06-10 23:29:27 guru Exp $
#
. ./functions.sh

pscoast -Rd -JN4.5i -Bg30/g15 -Dc -A10000 -Ggoldenrod -Ssnow2 -P > GMT_robinson.ps
