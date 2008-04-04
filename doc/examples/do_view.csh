#!/bin/csh
#
#	$Id: do_view.csh,v 1.4 2008-04-04 17:33:55 remko Exp $
#
#	Simple driver to view all examples using ghostview
#
if ($#argv == 1) then
	set viewer = $1
else
	set viewer = gv
endif

foreach f (ex??/example_*.ps)
	$viewer $f
end
