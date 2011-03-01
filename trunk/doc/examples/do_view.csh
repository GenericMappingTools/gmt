#!/bin/csh
#
#	$Id: do_view.csh,v 1.5 2011-03-01 01:34:48 remko Exp $
#
#	Simple driver to view all examples using ghostview
#
if ($#argv == 1) then
	set viewer = $1
else
	set viewer = gv
endif

foreach f (example_*.ps)
	$viewer $f
end
