#!/usr/bin/env bash
#
#
#	Simple driver to view all examples using ghostview
#
viewer=${1:-gv}
for f in ex*/ex*.ps
do
	$viewer $f
done
