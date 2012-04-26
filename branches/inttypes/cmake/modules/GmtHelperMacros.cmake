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
# add_file_to_cached_list (LIST [ FILE [ FILE ... ]])
# gmt_set_api_header (API_HEADER FUNCTIONS)
# gen_gmt_progpurpose.h (VARIABLE FILE [ FILE... ])
# gen_gmt_prog_names_cases_h (GMT_PROGS)
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

	# add_file_to_cached_list (LIST [ FILE [ FILE ... ]])
	# if FILE is omitted then the list is cleared
	# if FILE is not absolute then it is assumed to be in CMAKE_CURRENT_SOURCE_DIR
	# example: add_file_to_cached_list (list file)
	macro (ADD_FILE_TO_CACHED_LIST _LIST)
		set (_files ${ARGN})
		if (_files)
			set (_files_abs)
			foreach (_file ${_files})
				if (_file MATCHES "^[^/]")
					# make absolute path
					file(RELATIVE_PATH _file / ${CMAKE_CURRENT_SOURCE_DIR}/${_file})
				endif ()
				list(APPEND _files_abs /${_file})
			endforeach (_file ${_files})
			# append to list
			set (${_LIST} ${${_LIST}} ${_files_abs}
				CACHE INTERNAL "Global list of files")
		else (_theList)
			# clear list
			set (${_LIST} "" CACHE INTERNAL "Global list of files cleared")
		endif (_files)
	endmacro (ADD_FILE_TO_CACHED_LIST)

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
		configure_file (${GMT_SOURCE_DIR}/src/gmt_api.h.in
			gmt_${GMT_SUPPL_STRING}.h)
		set (GMT_SUPPL_STRING) # reset GMT_SUPPL_STRING
	endmacro (GMT_SET_API_HEADER _API_HEADER _FUNCTIONS)

	# gen_gmt_progpurpose.h (VARIABLE FILE [ FILE... ])
  # example: gen_gmt_progpurpose_h (GMT_PROG_PURPOSE ${GMT_PROGS_SRCS} ${GMT_PROGSPS_SRCS})
	macro (gen_gmt_progpurpose_h _PURPOSES _FILE)
		grep (
			"%s [API] -"
			_raw_purpose_list
			${_FILE} ${ARGN}
			LITERALLY
			)
		list_regex_replace (
			"^[ \t]*GMT_message \\\\(GMT, ([^ ]+)[^-]*(.*#Bn)#Bn.+"
			"\\\\1 \\\\2\""
			_purpose_list ${_raw_purpose_list})
		string_unescape (_purpose_list "${_purpose_list}" NOESCAPE_SEMICOLON)
		string (REPLACE ";" "\n  " _purpose_list "${_purpose_list}")
		set (${_PURPOSES} "${_purpose_list}")
		configure_file (gmt_progpurpose.h.in gmt_progpurpose.h)
	endmacro (gen_gmt_progpurpose_h _PURPOSES _FILE)

	# gen_gmt_prog_names_cases_h (GMT_PROGS)
	# gen_gmt_prog_names_cases_h (${_allGMT_PROGS})
	macro (gen_gmt_prog_names_cases_h GMT_PROGS)
		# gmt_prognames.h
		list_regex_replace (
			"^([^# \t:]+):([^ \t]+)"
			"{\"\\\\1\", \\\\2}"
			_prognames ${GMT_PROGS} ${ARGN}
			MATCHES_ONLY)
		list (REMOVE_DUPLICATES _prognames)
		string (REPLACE ";" ",\n" _prognames "${_prognames}")
		file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/gmt_prognames.h "${_prognames}\n")

		# gmt_progcases.h
		list_regex_replace (
			"^([^# \t:]+):([^ \t]+)"
			"\t\t\tfunc = (PFL)GMT_\\\\1#S\n\t\t\t*mode = \\\\2#S\n\t\t\tbreak#S"
			_raw_progcases ${GMT_PROGS} ${ARGN}
			MATCHES_ONLY)
		list (REMOVE_DUPLICATES _raw_progcases)
		set (_progcases)
		set (_casenum 0)
		foreach (_case ${_raw_progcases})
			list (APPEND _progcases "\t\tcase ${_casenum}:\n${_case}")
			math (EXPR _casenum "${_casenum} + 1")
		endforeach (_case ${_raw_progcases})
		string (REPLACE ";" "\n" _progcases "${_progcases}")
		string_unescape (_progcases "${_progcases}" NOESCAPE_SEMICOLON)
		file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/gmt_progcases.h "${_progcases}\n")
	endmacro (gen_gmt_prog_names_cases_h)


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
