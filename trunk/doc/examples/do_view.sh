#!/bin/sh
#
#	$Id: do_view.sh,v 1.2 2005-06-27 07:12:06 pwessel Exp $
#
#	Simple driver to view all examples using ghostview
#
for f in ex??/example_??.ps
do
	gv $f
done
