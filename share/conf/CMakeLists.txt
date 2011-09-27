# 
#	$Id$
#
#	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
#	See LICENSE.TXT file for copying and redistribution conditions.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; version 2 or any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
#
# share/conf/CMakeLists.txt
#

# Generate system defaults file
set (GMT_CONF_TEMPLATE "System-wide")
if (UNITS EQUAL "US")
	set (GMT_PS_CHAR_ENCODING "Standard+")
	set (GMT_PROJ_LENGTH_UNIT "inch")
	set (GMT_PS_MEDIA "letter")
else (UNITS EQUAL "US")
	set (GMT_PS_CHAR_ENCODING "ISOLatin1+")
	set (GMT_PROJ_LENGTH_UNIT "cm")
	set (GMT_PS_MEDIA "a4")
endif (UNITS EQUAL "US")
configure_file (gmt.conf.cmake gmt.conf)

set (GMT_CONF_TEMPLATE "Template US")
set (GMT_PS_CHAR_ENCODING "Standard+")
set (GMT_PROJ_LENGTH_UNIT "inch")
set (GMT_PS_MEDIA "letter")
configure_file (gmt.conf.cmake gmt_US.conf)

set (GMT_CONF_TEMPLATE "Template SI")
set (GMT_PS_CHAR_ENCODING "ISOLatin1+")
set (GMT_PROJ_LENGTH_UNIT "cm")
set (GMT_PS_MEDIA "a4")
configure_file (gmt.conf.cmake gmt_SI.conf)

set (GMT_CONFIG_FILES
	${CMAKE_CURRENT_BINARY_DIR}/gmt.conf
	${CMAKE_CURRENT_BINARY_DIR}/gmt_US.conf
	${CMAKE_CURRENT_BINARY_DIR}/gmt_SI.conf
)

install (FILES ${GMT_CONFIG_FILES} DESTINATION ${GMT_SHARE_PATH}/conf)
