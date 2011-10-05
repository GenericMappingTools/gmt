#
# $Id$
#
# Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
#

# source files
set (GMT_SEGY_LIB_SRCS segy_io.h segy.h segyreel.h segy_io.c)
set (GMT_SEGY_PROGS_SRCS pssegy.c pssegyz.c segy2grd.c)

# example files
set (GMT_SEGY_EXAMPLE_FILES README.segyprogs
	test.cpt test.list wa1_mig13.segy)
set (GMT_SEGY_EXAMPLE_PROGS segyprogs_1.sh segyprogs_2.sh segyprogs_3.sh
	segyprogs_4.sh segyprogs_5.sh)

# generate header file gmt_segy.h
gmt_set_api_header(GMT_SEGY_API_H "${GMT_SEGY_PROGS_SRCS}")

# make lib
add_library (gmt_segy ${GMT_SEGY_LIB_SRCS} ${GMT_SEGY_PROGS_SRCS} ${GMT_SEGY_API_H})
target_link_libraries (gmt_segy gmtlib)

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties (gmt_segy
	PROPERTIES
	VERSION "${GMT_PACKAGE_VERSION}"
	SOVERSION ${GMT_PACKAGE_VERSION_MAJOR}
	INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
	DEFINE_SYMBOL "LIBRARY_EXPORTS"
)

# create build targets
string (REPLACE ".c" "" GMT_SEGY_PROGS "${GMT_SEGY_PROGS_SRCS}")
foreach (prog ${GMT_SEGY_PROGS})
	add_executable (${prog} ${GMT_PROGRAM})
	set_target_properties (${prog}
	PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMT;FUNC=GMT_${prog}")
	target_link_libraries (${prog} gmt_segy)
endforeach (prog)

# generate Mac debugging symbols
create_debug_sym (gmt_segy ${GMT_SEGY_PROGS})

# add the install targets
install (TARGETS gmt_segy ${GMT_SEGY_PROGS}
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)

# add to gmt_suppl target
add_depend_to_target (gmt_suppl gmt_segy ${GMT_SEGY_PROGS})

# Generate manpages
string (REPLACE ".c" ".1" GMT_SEGY_MAN_1 "${GMT_SEGY_PROGS_SRCS}")
gmt_create_manpages ("${GMT_SEGY_MAN_1}")

# install example data
if (UNIX)
	install (FILES ${GMT_SEGY_EXAMPLE_FILES}
		DESTINATION
		${GMT_SHARE_PATH}/doc/gmt-${GMT_PACKAGE_VERSION}/supplements/segy)
	install (PROGRAMS ${GMT_SEGY_EXAMPLE_PROGS}
		DESTINATION
		${GMT_SHARE_PATH}/doc/gmt-${GMT_PACKAGE_VERSION}/supplements/segy)
endif (UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
