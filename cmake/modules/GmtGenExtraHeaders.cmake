#
#
# - Generates extra header files
# GMT_CREATE_HEADERS ()
#
# Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
	file2list (_file_lines ${GMT_SRC}/src/gmt_datasets.h)
	list (REMOVE_DUPLICATES _file_lines)
	list (LENGTH _file_lines GMT_N_DATASETS)

	# count lines in generated headers
	file2list (_file_lines gmt_keycases.h)
	list (LENGTH _file_lines GMT_N_KEYS)

	# gmt_dimensions.h
	configure_file (${GMT_SRC}/src/gmt_dimensions.h.in gmt_dimensions.h)
endmacro (gen_gmt_dimensions_h)

# Get something done
if (GENERATE_COMMAND STREQUAL gen_gmt_keywords_h)
	gen_gmt_keywords_h ()
elseif (GENERATE_COMMAND STREQUAL gen_gmt_dimensions_h)
	gen_gmt_dimensions_h ()
elseif (DEFINED GENERATE_COMMAND)
	message (SEND_ERROR "Unknown command: ${GENERATE_COMMAND}")
endif (GENERATE_COMMAND STREQUAL gen_gmt_keywords_h)
