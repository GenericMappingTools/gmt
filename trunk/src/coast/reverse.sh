#!/bin/sh
#
#	$Id$
#
# Use to reverse a polygon
tail -r $1 > $$
mv -f $$ $1
