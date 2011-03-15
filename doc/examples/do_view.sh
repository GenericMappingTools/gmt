#!/bin/bash
#
#	$Id: do_view.sh,v 1.5 2011-03-15 02:06:31 guru Exp $
#
#	Simple driver to view all examples using ghostview
#
viewer=${1:-gv}
for f in example_*.ps
do
	$viewer $f
done
