#!/bin/sh
#
#	$Id: reverse.sh,v 1.1 2004-09-05 04:25:01 pwessel Exp $
#
# Use to reverse a polygon
tail -r $1 > $$
mv -f $$ $1
