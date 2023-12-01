#
# Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
# Contact info: www.generic-mapping-tools.org
#-------------------------------------------------------------------------------
##
##	Settings for GMT MexFunction shared library
##
# Note: Only activated if GMT_BUILD_GMTMEX is set in ConfigUserAdvanced.cmake

if (GMT_BUILD_GMTMEX)
	# Add the flag to select Matlab vs Octave
	add_definitions(-DGMT_MATLAB)
	# Add the mexfunction as a shared library
	add_library (GMTmexfunction SHARED gmtmex.c)
	# Set the Matlab include file directory
	target_include_directories (GMTmexfunction PUBLIC ${GMT_SOURCE_DIR}/src ${GMT_BINARY_DIR}/src ${Matlab_INCLUDE_DIRS})
	# Link with the Matlab and mex libraries and gmt library
	target_link_libraries (GMTmexfunction gmtlib ${Matlab_MX_LIBRARY} ${Matlab_MEX_LIBRARY})
	# Set output properties of shared library
	set_target_properties (GMTmexfunction
		PROPERTIES
		OUTPUT_NAME gmtmex
		RUNTIME_OUTPUT_NAME gmtmex
		SUFFIX ".${Matlab_MEX_EXTENSION}"
		PREFIX "")
	# Install gmt.m in the bin dir
	install (PROGRAMS gmt.m
		DESTINATION ${GMT_BINDIR}
		COMPONENT Runtime)
	# Install the gmtmex function in the bin dir instead of lib dir
	install (PROGRAMS "${GMT_BINARY_DIR}/src/gmtmex/gmtmex.${Matlab_MEX_EXTENSION}"
		DESTINATION ${GMT_BINDIR}
		COMPONENT Runtime)
endif (GMT_BUILD_GMTMEX)
