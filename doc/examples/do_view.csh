#!/bin/csh
#
#	$Id: do_view.csh,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
#
#	Simple driver to view all examples using ghostview
#
foreach f (ex??/example_??.ps)
	ghostview $f
end
