#
#
# - Generates extra header files
#
# Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

include (ManageString)

macro (gen_gmt_keywords_h)
	# gmt_keycases.h
	file2list (_gmtkeywords_file
		${GMT_SRC}/src/gmt_keywords.txt
		${GMT_SRC}/src/gmt_keywords.d)
	list_regex_replace (
		"^([^#\t ]+).*"
		"\\\\1 "
		_gmtkeycases ${_gmtkeywords_file}
		MATCHES_ONLY)
	list (SORT _gmtkeycases)
	list (REMOVE_DUPLICATES _gmtkeycases)
	set (_gmtkeycasedef)
	set (_casenum 0)
	foreach (_case ${_gmtkeycases})
		list (APPEND _gmtkeycasedef "#define GMTCASE_${_case}${_casenum}")
		math (EXPR _casenum "${_casenum} + 1")
	endforeach (_case ${_gmtkeycases})
	string (REPLACE ";" "\n" _gmtkeycasedef "${_gmtkeycasedef}")
	file (WRITE gmt_keycases.h "${_gmtkeycasedef}\n")

	# gmt_keywords.h
	list_regex_replace (
		"^([^\t ]+)[\t ]*$"
		"\"\\\\1\""
		_gmtkeywords ${_gmtkeycases})
	string (REPLACE ";" ",\n" _gmtkeywords "${_gmtkeywords}")
	file (WRITE gmt_keywords.h "${_gmtkeywords}\n")
endmacro (gen_gmt_keywords_h)

# gmt_media_name.h gmt_pennames.h gmt_unique.h gmt_cpt_masters.h
macro (gen_gmt_dimensions_h)
	file2list (_file_lines ${GMT_SRC}/src/gmt_media_name.h)
	list (REMOVE_DUPLICATES _file_lines)
	list (LENGTH _file_lines GMT_N_MEDIA)
	file2list (_file_lines ${GMT_SRC}/src/gmt_pennames.h)
	list (REMOVE_DUPLICATES _file_lines)
	list (LENGTH _file_lines GMT_N_PEN_NAMES)
	file2list (_file_lines ${GMT_SRC}/src/gmt_unique.h)
	list (REMOVE_DUPLICATES _file_lines)
	list (LENGTH _file_lines GMT_N_UNIQUE)
	file2list (_file_lines ${GMT_SRC}/src/gmt_ellipsoids.h)
	list (LENGTH _file_lines GMT_N_ELLIPSOIDS)
	file2list (_file_lines ${GMT_SRC}/src/gmt_datums.h)
	list (LENGTH _file_lines GMT_N_DATUMS)
	file2list (_file_lines ${GMT_SRC}/src/gmt_colornames.h)
	list (LENGTH _file_lines GMT_N_COLOR_NAMES)
	file2list (_file_lines ${GMT_SRC}/src/gmt_cpt_masters.h)
	list (REMOVE_DUPLICATES _file_lines)
	list (LENGTH _file_lines GMT_N_CPT_MASTERS)

	# count lines in generated headers
	file2list (_file_lines gmt_keycases.h)
	list (LENGTH _file_lines GMT_N_KEYS)

	# gmt_dimensions.h
	configure_file (${GMT_SRC}/src/gmt_dimensions.h.in gmt_dimensions.h)
endmacro (gen_gmt_dimensions_h)

# CMake macro to generate gmt_${SHARED_LIB_NAME}_moduleinfo.h
macro (gen_gmt_moduleinfo_h output_header_file)
	set (_moduleinfo)
	foreach (_prog_src ${ARGN})
		file (READ ${GMT_SRC}/src/${_prog_src} _src_content)

		string (REGEX MATCH "#define[ \t]*THIS_MODULE_CLASSIC_NAME[ \t]*\"([^\n]*)\"[ \t]*\n" _this_module_classic_name ${_src_content})
		set (_this_module_classic_name ${CMAKE_MATCH_1})
		if ("${_this_module_classic_name}" STREQUAL "")
			message (WARNING "THIS_MODULE_CLASSIC_NAME is empty in ${_prog_src}")
		endif ("${_this_module_classic_name}" STREQUAL "")

		string (REGEX MATCH "#define[ \t]*THIS_MODULE_MODERN_NAME[ \t]*\"([^\n]*)\"[ \t]*\n" _this_module_modern_name ${_src_content})
		set (_this_module_modern_name ${CMAKE_MATCH_1})
		if ("${_this_module_modern_name}" STREQUAL "")
			message (WARNING "THIS_MODULE_MODERN_NAME is empty in ${_prog_src}")
		endif ("${_this_module_modern_name}" STREQUAL "")

		string (REGEX MATCH "#define[ \t]*THIS_MODULE_LIB[ \t]*\"([^\n]*)\"[ \t]*\n" _this_module_lib ${_src_content})
		set (_this_module_lib ${CMAKE_MATCH_1})
		if ("${_this_module_lib}" STREQUAL "")
			message (WARNING "THIS_MODULE_LIB is empty in ${_prog_src}")
		endif ("${_this_module_lib}" STREQUAL "")

		string (REGEX MATCH "#define[ \t]*THIS_MODULE_PURPOSE[ \t]*\"([^\n]*)\"[ \t]*\n" _this_module_purpose ${_src_content})
		set (_this_module_purpose ${CMAKE_MATCH_1})
		if ("${_this_module_purpose}" STREQUAL "")
			message (WARNING "THIS_MODULE_PURPOSE is empty in ${_prog_src}")
		endif ("${_this_module_purpose}" STREQUAL "")

		string (REGEX MATCH "#define[ \t]*THIS_MODULE_KEYS[ \t]*\"([^\n]*)\"[ \t]*\n" _this_module_keys ${_src_content})
		set (_this_module_keys ${CMAKE_MATCH_1})

		set (_this_module_info "\t{\"${_this_module_modern_name}\", \"${_this_module_classic_name}\", \"${_this_module_lib}\", \"${_this_module_purpose}\", \"${_this_module_keys}\"},")
		if (_moduleinfo)
			set (_moduleinfo "${_moduleinfo}\n${_this_module_info}")
		else (_moduleinfo)
			set (_moduleinfo "${_this_module_info}")
		endif (_moduleinfo)
	endforeach (_prog_src ${ARGN})
	file (WRITE "${CMAKE_CURRENT_BINARY_DIR}/${output_header_file}" ${_moduleinfo})
endmacro (gen_gmt_moduleinfo_h)

# Get something done
if (GENERATE_COMMAND STREQUAL gen_gmt_keywords_h)
	gen_gmt_keywords_h ()
elseif (GENERATE_COMMAND STREQUAL gen_gmt_dimensions_h)
	gen_gmt_dimensions_h ()
elseif (GENERATE_COMMAND STREQUAL gen_gmt_moduleinfo_h)
	separate_arguments (PROGS_SRCS)
	gen_gmt_moduleinfo_h (${OUTPUT_HEADER_FILE} "${PROGS_SRCS}")
elseif (DEFINED GENERATE_COMMAND)
	message (SEND_ERROR "Unknown command: ${GENERATE_COMMAND}")
endif (GENERATE_COMMAND STREQUAL gen_gmt_keywords_h)
