#!/bin/csh
#
#	$Id: do_view.csh,v 1.2 2004-05-26 22:59:16 pwessel Exp $
#
#	Simple driver to view all examples using ghostview
#
if ($#argv == 1) then
	set viewer = $1
else
	set viewer = ghostview
endif

foreach f (ex??/example_??.ps)
	$viewer $f
end
