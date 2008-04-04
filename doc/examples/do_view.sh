#!/bin/sh
#
#	$Id: do_view.sh,v 1.4 2008-04-04 17:33:55 remko Exp $
#
#	Simple driver to view all examples using ghostview
#
viewer=${1:-gv}
for f in ex??/example_*.ps
do
	$viewer $f
done
