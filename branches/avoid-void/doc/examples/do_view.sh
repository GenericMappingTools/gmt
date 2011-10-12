#!/bin/bash
#
#	$Id$
#
#	Simple driver to view all examples using ghostview
#
viewer=${1:-gv}
for f in example_*.ps
do
	$viewer $f
done
