#
# $Id$
#
# - Usefull CMake macros
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

# tag_from_current_source_dir (TAG [PREFIX])
# add_depend_to_target (TARGET DEPEND [ DEPEND [ DEPEND ... ]])
# add_depend_to_spotless (DEPEND [ DEPEND [ DEPEND ... ]])
# gmt_set_api_header (API_HEADER FUNCTIONS)
# get_subdir_var (VARIABLE VAR_NAME DIR [ DIR ... ])
# get_subdir_var_files (VARIABLE VAR_NAME DIR [ DIR ... ])

if(NOT DEFINED _GMT_HELPER_MACROS_CMAKE_)
	set(_GMT_HELPER_MACROS_CMAKE_ "DEFINED")

	# tag_from_current_source_dir (TAG [PREFIX])
	# example: tag_from_current_source_dir (_tag "_")
	macro (TAG_FROM_CURRENT_SOURCE_DIR _TAG)
		get_filename_component (_basename ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)
		string(COMPARE NOTEQUAL "${_basename}" "src" _in_subtree)
		set (${_TAG})
		if (_in_subtree)
			set (${_TAG} "${ARGN}${_basename}")
		endif (_in_subtree)
	endmacro (TAG_FROM_CURRENT_SOURCE_DIR)


	# add_depend_to_target (TARGET DEPEND [ DEPEND [ DEPEND ... ]])
	# example: add_depend_to_target (main_target custom_target)
	macro (ADD_DEPEND_TO_TARGET _TARGET)
		if(NOT TARGET ${_TARGET})
			add_custom_target (${_TARGET}
				WORKING_DIRECTORY ${GMT_BINARY_DIR})
		endif(NOT TARGET ${_TARGET})
		add_dependencies(${_TARGET} ${ARGN})
	endmacro (ADD_DEPEND_TO_TARGET)


	# add_depend_to_spotless (DEPEND [ DEPEND [ DEPEND ... ]])
	# example: add_depend_to_spotless (custom_target)
	macro (ADD_DEPEND_TO_SPOTLESS)
		if(NOT TARGET spotless)
			add_custom_target (spotless
				COMMAND make clean
				WORKING_DIRECTORY ${GMT_BINARY_DIR})
		endif(NOT TARGET spotless)
		add_dependencies(spotless ${ARGV})
	endmacro (ADD_DEPEND_TO_SPOTLESS)


	# gmt_set_api_header (API_HEADER FUNCTIONS)
	# example: gmt_set_api_header(GMT_MECA_API_H "${GMT_MECA_PROGS_SRCS}")
	macro (GMT_SET_API_HEADER _API_HEADER _FUNCTIONS)
		string (REPLACE ".c" "" _functions "${_FUNCTIONS};${ARGN}")
		if (NOT GMT_SUPPL_STRING)
			tag_from_current_source_dir (GMT_SUPPL_STRING)
		endif (NOT GMT_SUPPL_STRING)
		set (${_API_HEADER} gmt_${GMT_SUPPL_STRING}.h)
		string (TOUPPER ${GMT_SUPPL_STRING} GMT_SUPPL_STRING_UPPER)
		set (GMT_API_FUNCTION_LIST) # reset list
		foreach (_function ${_functions})
			set (_api_function "EXTERN_MSC GMT_LONG GMT_${_function} (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)")
			list (APPEND GMT_API_FUNCTION_LIST ${_api_function})
		endforeach (_function)
		string (REPLACE ";" ";\n" GMT_API_FUNCTION_LIST "${GMT_API_FUNCTION_LIST}")
		# create api header file from template
		configure_file (${GMT_SOURCE_DIR}/src/gmt_api.h.cmake
			gmt_${GMT_SUPPL_STRING}.h)
		set (${_API_HEADER} gmt_${GMT_SUPPL_STRING}.h)
		set (GMT_SUPPL_STRING) # reset GMT_SUPPL_STRING
	endmacro (GMT_SET_API_HEADER _API_HEADER _FUNCTIONS)

	# get_subdir_var (VARIABLE VAR_NAME DIR [ DIR ... ])
	# example: get_subdir_var (SUB_TARGETS PROGS ${SUB_DIRS})
	macro (GET_SUBDIR_VAR VARIABLE VAR_NAME DIR_NAME)
		set (${VARIABLE}) # clear VARIABLE
		foreach (_dir ${DIR_NAME} ${ARGN})
			# get value of variable ${VAR_NAME} in dir ${_dir}:
			get_directory_property (_value DIRECTORY ${_dir} DEFINITION ${VAR_NAME})
			list (APPEND ${VARIABLE} "${_value}")
		endforeach(_dir)
	endmacro (GET_SUBDIR_VAR VARIABLE VAR_NAME DIR_NAME)

	# get_subdir_var_files (VARIABLE VAR_NAME DIR [ DIR ... ])
	# example: get_subdir_var_files (SUB_LIB_SRCS LIB_SRCS ${SUB_DIRS})
	macro (GET_SUBDIR_VAR_FILES VARIABLE VAR_NAME DIR_NAME)
		set (${VARIABLE}) # clear VARIABLE
		foreach (_dir ${DIR_NAME} ${ARGN})
			# get value of variable ${VAR_NAME} in dir ${_dir}:
			get_directory_property (_files DIRECTORY ${_dir} DEFINITION ${VAR_NAME})
			foreach (_file ${_files})
				# prepend dirname:
				list (APPEND ${VARIABLE} "${_dir}/${_file}")
			endforeach (_file)
		endforeach(_dir)
	endmacro (GET_SUBDIR_VAR_FILES VARIABLE VAR_NAME DIR_NAME)

endif(NOT DEFINED _GMT_HELPER_MACROS_CMAKE_)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
