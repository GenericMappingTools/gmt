#
# $I$
#
# Copyright (c) 1991-2016 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; version 3 or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------

set (GMT_TUTORIAL_FILES bermuda.nc data otemp.anal1deg.nc quakes.cpt
	quakes.ngdc ship.xyz topo.cpt us.nc)
	
install (FILES ${GMT_TUTORIAL_FILES}
	DESTINATION ${GMT_DOCDIR}/tutorial
	COMPONENT Documentation)
