#
# $Id$
#
# - Useful CMake macros
#
# Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

# tag_from_current_source_dir (TAG [PREFIX])
# add_depend_to_target (TARGET DEPEND [ DEPEND [ DEPEND ... ]])
# add_file_to_cached_list (LIST [ FILE [ FILE ... ]])
# get_subdir_var (VARIABLE VAR_NAME DIR [ DIR ... ])
# get_subdir_var_files (VARIABLE VAR_NAME DIR [ DIR ... ])
# install_module_symlink (MODULE [ MODULE ... ])

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

	# install_module_symlink (MODULE [ MODULE ... ])
	# example: install_module_symlink (grdimage psxy)
	if (NOT DEFINED GMT_INSTALL_MODULE_LINKS)
		# If not defined or set to TRUE
		set (GMT_INSTALL_MODULE_LINKS TRUE)
	endif ()
	macro (INSTALL_MODULE_SYMLINK)
		if (WIN32 AND GMT_INSTALL_MODULE_LINKS)
			# create build targets
			foreach (_gmtmodule ${ARGV})
				add_executable (${_gmtmodule} ${GMT_PROGRAM})
				string (TOUPPER "${_gmtmodule}" UCASEname)
				set_target_properties (${_gmtmodule} PROPERTIES
					COMPILE_DEFINITIONS "MODULE=\"${_gmtmodule}\"")
				target_link_libraries (${_gmtmodule} gmtlib)
			endforeach (_gmtmodule)

			# add the install targets
			install (TARGETS ${ARGV}
				RUNTIME DESTINATION ${GMT_BINDIR}
				COMPONENT Runtime)

			# add to gmt_suppl target
			add_depend_to_target (gmt_module_progs ${ARGV})
		elseif (UNIX AND GMT_INSTALL_MODULE_LINKS)
			# create gmt module symlinks to gmt
			foreach (_gmtmodule ${ARGV})
				install (CODE "
				execute_process (COMMAND ${CMAKE_COMMAND} -E remove -f
					\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${GMT_BINDIR}/${_gmtmodule})
				execute_process (COMMAND ${CMAKE_COMMAND} -E create_symlink
					gmt${GMT_INSTALL_NAME_SUFFIX}
					\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${GMT_BINDIR}/${_gmtmodule})
				" COMPONENT Runtime)
			endforeach (_gmtmodule)
		endif (WIN32 AND GMT_INSTALL_MODULE_LINKS)
	endmacro (INSTALL_MODULE_SYMLINK)

endif(NOT DEFINED _GMT_HELPER_MACROS_CMAKE_)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
