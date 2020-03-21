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

# Macro definition
if(NOT DEFINED _COPY_DIR_IF_DIFFERENT_CMAKE_)
	set(_COPY_DIR_IF_DIFFERENT_CMAKE_ "DEFINED")

	# copy_dir_if_different (TARGET SOURCE DEST)
	# example: copy_dir_if_different (copy_target srcdir/ destdir)
	macro (COPY_DIR_IF_DIFFERENT _TARGET _SOURCE _DEST)
		file(RELATIVE_PATH _rel_source ${CMAKE_SOURCE_DIR} ${_SOURCE})
		add_custom_target (${_TARGET}
			COMMAND ${CMAKE_COMMAND}
			-D SOURCE=${_SOURCE}
			-D DESTINATION=${_DEST}
			-D INVOKE_COPY_DIR_IF_DIFFERENT=TRUE
			-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
			-P ${CMAKE_MODULE_PATH}/CopyDirIfDifferent.cmake
			COMMENT "Copying ${_rel_source} ..."
			DEPENDS ${_SOURCE})
	endmacro (COPY_DIR_IF_DIFFERENT)
endif(NOT DEFINED _COPY_DIR_IF_DIFFERENT_CMAKE_)

# Copy source directory recursively
# Preserve file permissions and timestamps
if (DEFINED INVOKE_COPY_DIR_IF_DIFFERENT)
	file (COPY ${SOURCE} DESTINATION ${DESTINATION})
endif (DEFINED INVOKE_COPY_DIR_IF_DIFFERENT)
