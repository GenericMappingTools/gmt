#!/bin/sh
#
#	$Id: do_view.sh,v 1.3 2007-09-13 17:35:56 remko Exp $
#
#	Simple driver to view all examples using ghostview
#
viewer=${1:-gv}
for f in ex??/example_??.ps
do
	$viewer $f
done
