#
#
# Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
#
# share CMakeLists.txt
#

set (_gmt_share_dirs cpt custom localization mgd77 mgg
	spotter x2sys)

# install target for data
install (DIRECTORY ${_gmt_share_dirs}
	DESTINATION ${GMT_DATADIR}
	COMPONENT Runtime
	PATTERN "CMakeLists.txt" EXCLUDE
	REGEX "[.](cmake|in)$" EXCLUDE)

# only attempt to install shorelines when requested and path is known
if (GSHHG_PATH AND COPY_GSHHG)
	install (DIRECTORY ${GSHHG_PATH}/
		DESTINATION ${GMT_DATADIR}/coast
		COMPONENT GSHHG)
	# location must be blank in gmt.conf
	set (GSHHG_INSTALL_PATH)
	# create coastline.conf which is needed for in-build-dir ctest
	configure_file (coastline.conf.in coastline.conf @ONLY)
else (GSHHG_PATH AND COPY_GSHHG)
	# set installed location
	set (GSHHG_INSTALL_PATH ${GSHHG_PATH})
endif (GSHHG_PATH AND COPY_GSHHG)

# only attempt to install DCW file when requested and path is known
if (DCW_PATH AND COPY_DCW)
	install (DIRECTORY ${DCW_PATH}/
		DESTINATION ${GMT_DATADIR}/dcw
		COMPONENT DCW)
	# location must be blank in gmt.conf
	set (DCW_INSTALL_PATH)
else (DCW_PATH AND COPY_DCW)
	# set installed location
	set (DCW_INSTALL_PATH ${DCW_PATH})
endif (DCW_PATH AND COPY_DCW)

# add subtrees
add_subdirectory (tools)
