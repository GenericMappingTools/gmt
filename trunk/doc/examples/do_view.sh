#!/bin/sh
#
#	$Id: do_view.sh,v 1.1 2001-09-14 18:55:04 pwessel Exp $
#
#	Simple driver to view all examples using ghostview
#
for f in ex??/example_??.ps
do
	ghostview $f
done
