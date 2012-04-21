#
# $Id$
#
# Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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
#

# xgrid is GPL only
if (LICENSE_RESTRICTED AND NOT LICENSE_RESTRICTED STREQUAL GPL)
	return()
endif (LICENSE_RESTRICTED AND NOT LICENSE_RESTRICTED STREQUAL GPL)

if (DEFINED XGRID AND NOT XGRID)
	return()
endif (DEFINED XGRID AND NOT XGRID)

include (FindX11)

if (X11_FOUND AND X11_Xt_FOUND)
	find_library(XAW_LIBRARY
		NAMES Xaw Xaw7
		PATHS ${X11_LIB_SEARCH_PATH})
endif (X11_FOUND AND X11_Xt_FOUND)

if (X11_FOUND AND X11_Xt_FOUND AND XAW_LIBRARY)

	# source files
	set (GMT_XGRID_PROGS_SRCS xgrid_Canvas.h xgrid_CanvasP.h xgrid_GMTgrid.h
		xgrid_Panner.h xgrid_PannerP.h xgrid_Xutility.h xgrid_controls.h
		xgrid_grid.h xgrid_messages.h xgrid_textInput.h xgrid_textView.h
		xgrid_utility.h xgrid_view.h xgrid_utility.c xgrid_Xutility.c
		xgrid_Canvas.c xgrid_Panner.c xgrid_controls.c xgrid_messages.c
		xgrid_GMTgrid.c xgrid_view.c xgrid_textView.c xgrid_textInput.c
		xGridEdit.c)

	include_directories (${X11_INCLUDE_DIR})
	#add_definitions (${NETCDF_DEFINITIONS})

	# build target
	add_executable (xgridedit ${GMT_XGRID_PROGS_SRCS})
	target_link_libraries (xgridedit
		gmtlib
		${X11_LIBRARIES}
		${X11_Xt_LIB}
		${XAW_LIBRARY})

	# generate Mac debugging symbols
	create_debug_sym (xgridedit)

	# add the install targets
	install (TARGETS xgridedit
		RUNTIME DESTINATION ${GMT_BINDIR}
		COMPONENT Runtime)

	# add to gmt_suppl target
	add_depend_to_target (gmt_suppl xgridedit)

endif (X11_FOUND AND X11_Xt_FOUND AND XAW_LIBRARY)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
