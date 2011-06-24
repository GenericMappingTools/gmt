#!/bin/bash
#	$Id: GMT_robinson.sh,v 1.9 2011-06-24 21:13:45 guru Exp $
#
. ./functions.sh

pscoast -Rd -JN4.5i -Bg -Dc -A10000 -Ggoldenrod -Ssnow2 -P > GMT_robinson.ps
