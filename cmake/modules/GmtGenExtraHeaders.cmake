#
# $Id$
#
# - Generates extra header files
# GMT_CREATE_HEADERS ()
#
# Copyright (c) 1991-2019 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

include (ManageString)

# gmt_datums.h
macro (gen_gmt_datums_h)
	file2list (_datums_file ${GMT_SRC}/src/Datums.txt)
	list_regex_replace (
		"^([^#\t]+)[\t]+([^\t]+)[\t]+([^\t]+)[\t]+([^\t]+)[\t]+([^\t]+)[\t]+(.+)"
		"\t\t{\"\\\\1\", \"\\\\2\", \"\\\\6\", {\\\\3, \\\\4, \\\\5}}"
		_datums ${_datums_file}
		MATCHES_ONLY)
	list (REMOVE_DUPLICATES _datums)
	string (REPLACE ";" ",\n" _datums "${_datums}")
	string_unescape (_datums "${_datums}" NOESCAPE_SEMICOLON)
	file (WRITE gmt_datums.h "${_datums}\n")
endmacro (gen_gmt_datums_h)

macro (gen_gmt_colors_h)
	# gmt_colornames.h
	file2list (_color_file ${GMT_SRC}/src/Colors.txt)
	list_regex_replace (
		"^[0-9 \t]+([a-z]+[0-9]*).*"
		"\"\\\\1\""
		_color_names ${_color_file}
		MATCHES_ONLY)
	string (REPLACE ";" ",\n" _color_names "${_color_names}")
	file (WRITE gmt_colornames.h "${_color_names}\n")

	# gmt_color_rgb.h
	list_regex_replace (
		"^[ ]*([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+).*"
		"{\\\\1, \\\\2, \\\\3}"
		_colors_rgb ${_color_file}
		MATCHES_ONLY)
	string (REPLACE ";" ",\n" _colors_rgb "${_colors_rgb}")
	file (WRITE gmt_color_rgb.h "${_colors_rgb}\n")

#	# Colors.i
#	list_regex_replace (
#		"^([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+([a-z]+[0-9]*).*"
#		"\\\\1\t\\\\2\t\\\\3\t\\\\4"
#		_colors_man ${_color_file}
#		MATCHES_ONLY)
#	string (REPLACE ";" "\n.br\n" _colors_man "${_colors_man}")
#	file (WRITE Colors.i ".br\n${_colors_man}\n")
endmacro (gen_gmt_colors_h)

# Fonts.i
#macro (gen_ps_font_info)
#	file2list (_fonts_file ${GMT_SRC}/share/pslib/PS_font_info.d)
#	list_regex_replace (
#		"^([^# \t]+).*"
#		"\\\\1"
#		_fonts_list ${_fonts_file}
#		MATCHES_ONLY)
#	set (_fonts_man)
#	set (_fontnum 0)
#	foreach (_font ${_fonts_list})
#		list (APPEND _fonts_man "${_fontnum}\t${_font}")
#		math (EXPR _fontnum "${_fontnum} + 1")
#	endforeach (_font ${_fonts_list})
#	string (REPLACE ";" "\n.br\n" _fonts_man "${_fonts_man}")
#	file (WRITE Fonts.i ".br\n${_fonts_man}\n")
#endmacro (gen_ps_font_info)

macro (gen_gmt_ellipsoids)
	# gmt_ellipsoids.h
	file2list (_ellipsnames_file ${GMT_SRC}/src/Ellipsoids.txt)
	list_regex_replace (
		"^([^#\t]+)[ \t]+([0-9]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+:.*"
		"\t\t{\"\\\\1\", \\\\2, \\\\3, 1.0/\\\\4}"
		_ellipsnames ${_ellipsnames_file}
		MATCHES_ONLY)
	list (REMOVE_DUPLICATES _ellipsnames)
	string (REPLACE "1.0/0}" "0}" _ellipsnames "${_ellipsnames}")
	string (REPLACE ";" ",\n" _ellipsnames "${_ellipsnames}")
	file (WRITE gmt_ellipsoids.h "${_ellipsnames}\n")

#	# Ellipsoids.i
#	list_regex_replace (
#		"^([^#\t]+)[ \t]+([0-9]+)[ \t]+[0-9.]+[ \t]+[0-9.]+[ \t]+:[ \t]+(.*)"
#		"\\\\1: \\\\3 (\\\\2)"
#		_ellipsnames ${_ellipsnames_file}
#		MATCHES_ONLY)
#	list (REMOVE_DUPLICATES _ellipsnames)
#	string (REPLACE ";" "\n.br\n" _ellipsnames "${_ellipsnames}")
#	file (WRITE Ellipsoids.i "${_ellipsnames}\n")
endmacro (gen_gmt_ellipsoids)

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
	file2list (_file_lines ${GMT_SRC}/src/gmt_cpt_masters.h)
	list (REMOVE_DUPLICATES _file_lines)
	list (LENGTH _file_lines GMT_N_CPT_MASTERS)

	# count lines in generated heders
	file2list (_file_lines gmt_datums.h)
	list (LENGTH _file_lines GMT_N_DATUMS)
	file2list (_file_lines gmt_ellipsoids.h)
	list (LENGTH _file_lines GMT_N_ELLIPSOIDS)
	file2list (_file_lines gmt_colornames.h)
	list (LENGTH _file_lines GMT_N_COLOR_NAMES)
	file2list (_file_lines gmt_keycases.h)
	list (LENGTH _file_lines GMT_N_KEYS)

	# gmt_dimensions.h
	configure_file (${GMT_SRC}/src/gmt_dimensions.h.in gmt_dimensions.h)
endmacro (gen_gmt_dimensions_h)

# gmtmath.h gmtmath_op.h gmtmath_explain.h gmtmath_man.i
# GMTMATH_OPERATOR_INIT GMTMATH_OPERATOR_ARRAY GMTMATH_OPERATOR_EXPLAIN
# GMTMATH_N_OPERATORS
macro (gen_gmt_math_h)
	grep (
		"^/[* ]+OPERATOR:"
		_raw_op_descriptions
		${GMT_SRC}/src/gmtmath.c)

	# gmtmath.h
	list_regex_replace (
		"^[^:]+:[ ]+([^ ]+)[ ]+([0-9]+)[ ]+([0-9]+)[ ]+(.+)[.][ *]+/.*"
		"\tops[OP_NUM] = table_\\\\1#S\tn_args[OP_NUM] = \\\\2#S\tn_out[OP_NUM] = \\\\3#S"
		_raw_op_init ${_raw_op_descriptions}
		MATCHES_ONLY)
	list (LENGTH _raw_op_init GMTMATH_N_OPERATORS)
	set (_op_init)
	set (_opnum 0)
	foreach (_op ${_raw_op_init})
		string (REPLACE "OP_NUM" "${_opnum}" _op ${_op})
		list (APPEND _op_init ${_op})
		math (EXPR _opnum "${_opnum} + 1")
	endforeach (_op ${_raw_op_init})
	string (REPLACE ";" "\n" _op_init "${_op_init}")
	string_unescape (GMTMATH_OPERATOR_INIT "${_op_init}" NOESCAPE_SEMICOLON)

	configure_file (${GMT_SRC}/src/gmtmath.h.in gmtmath.h)

	# gmtmath_op.h
	list_regex_replace (
		"^[^:]+:[ ]+([^ ]+)[ ]+([0-9]+)[ ]+([0-9]+)[ ]+(.+)[.][ *]+/.*"
		"\t\"\\\\1\",\t/* id = OP_NUM */"
		_raw_op_array ${_raw_op_descriptions}
		MATCHES_ONLY)
	set (_op_array)
	set (_opnum 0)
	foreach (_op ${_raw_op_array})
		string (REPLACE "OP_NUM" "${_opnum}" _op ${_op})
		list (APPEND _op_array ${_op})
		math (EXPR _opnum "${_opnum} + 1")
	endforeach (_op ${_raw_op_array})
	string (REPLACE ";" "\n" GMTMATH_OPERATOR_ARRAY "${_op_array}")

	configure_file (${GMT_SRC}/src/gmtmath_op.h.in gmtmath_op.h)

	# gmtmath_explain.h
	list_regex_replace (
		"^[^:]+:[ ]+([^ ]+)[ ]+([0-9]+)[ ]+([0-9]+)[ ]+(.+)[.][ *]+/.*"
		"\t\t\"\t\\\\1PADDING\\\\2  \\\\3    \\\\4#Bn\""
		_op_explain ${_raw_op_descriptions}
		MATCHES_ONLY)
	set (_padded_lines)
	foreach (_line ${_op_explain})
		string_pad (_line 15 PADDING)
		list (APPEND _padded_lines ${_line})
	endforeach (_line ${_op_explain})
	string (REPLACE ";" "\n" _op_explain "${_padded_lines}")
	string_unescape (GMTMATH_OPERATOR_EXPLAIN "${_op_explain}" NOESCAPE_SEMICOLON)

	configure_file (${GMT_SRC}/src/gmtmath_explain.h.in gmtmath_explain.h)

#	# gmtmath_man.i
#	list_regex_replace (
#		"^[^:]+:[ ]+([^ ]+)[ ]+([0-9]+)[ ]+([0-9]+)[ ]+(.+)[.][ *]+/.*"
#		"#BfB\\\\1#BfP\t\\\\2  \\\\3\t\\\\4"
#		_op_man ${_raw_op_descriptions}
#		MATCHES_ONLY)
#	string (REPLACE ";" "\n" _op_man "${_op_man}")
#	string_unescape (_op_man "${_op_man}" NOESCAPE_SEMICOLON)
#	set (_op_man "Choose among the following ${GMTMATH_N_OPERATORS} operators.\n"
#	"\"args\" are the number of input and output arguments.\n"
#	".TS\nl l l .\nOperator\targs\tReturns\n${_op_man}\n.TE\n")
#	file (WRITE gmtmath_man.i ${_op_man})
endmacro (gen_gmt_math_h)

# grdmath.h grdmath_op.h grdmath_explain.h grdmath_man.i
# GRDMATH_OPERATOR_INIT GRDMATH_OPERATOR_ARRAY GRDMATH_OPERATOR_EXPLAIN
# GRDMATH_N_OPERATORS
macro (gen_grd_math_h)
	grep (
		"^/[* ]+OPERATOR:"
		_raw_op_descriptions
		${GMT_SRC}/src/grdmath.c)

	# grdmath.h
	list_regex_replace (
		"^[^:]+:[ ]+([^ ]+)[ ]+([0-9]+)[ ]+([0-9]+)[ ]+(.+)[.][ *]+/.*"
		"\tops[OP_NUM] = grd_\\\\1#S\tn_args[OP_NUM] = \\\\2#S\tn_out[OP_NUM] = \\\\3#S"
		_raw_op_init ${_raw_op_descriptions}
		MATCHES_ONLY)
	list (LENGTH _raw_op_init GRDMATH_N_OPERATORS)
	set (_op_init)
	set (_opnum 0)
	foreach (_op ${_raw_op_init})
		string (REPLACE "OP_NUM" "${_opnum}" _op ${_op})
		list (APPEND _op_init ${_op})
		math (EXPR _opnum "${_opnum} + 1")
	endforeach (_op ${_raw_op_init})
	string (REPLACE ";" "\n" _op_init "${_op_init}")
	string_unescape (GRDMATH_OPERATOR_INIT "${_op_init}" NOESCAPE_SEMICOLON)

	configure_file (${GMT_SRC}/src/grdmath.h.in grdmath.h)

	# grdmath_op.h
	list_regex_replace (
		"^[^:]+:[ ]+([^ ]+)[ ]+([0-9]+)[ ]+([0-9]+)[ ]+(.+)[.][ *]+/.*"
		"\t\"\\\\1\",\t/* id = OP_NUM */"
		_raw_op_array ${_raw_op_descriptions}
		MATCHES_ONLY)
	set (_op_array)
	set (_opnum 0)
	foreach (_op ${_raw_op_array})
		string (REPLACE "OP_NUM" "${_opnum}" _op ${_op})
		list (APPEND _op_array ${_op})
		math (EXPR _opnum "${_opnum} + 1")
	endforeach (_op ${_raw_op_array})
	string (REPLACE ";" "\n" GRDMATH_OPERATOR_ARRAY "${_op_array}")

	configure_file (${GMT_SRC}/src/grdmath_op.h.in grdmath_op.h)

	# grdmath_explain.h
	list_regex_replace (
		"^[^:]+:[ ]+([^ ]+)[ ]+([0-9]+)[ ]+([0-9]+)[ ]+(.+)[.][ *]+/.*"
		"\t\t\"\t\\\\1PADDING\\\\2  \\\\3    \\\\4#Bn\""
		_op_explain ${_raw_op_descriptions}
		MATCHES_ONLY)
	set (_padded_lines)
	foreach (_line ${_op_explain})
		string_pad (_line 15 PADDING)
		list (APPEND _padded_lines ${_line})
	endforeach (_line ${_op_explain})
	string (REPLACE ";" "\n" _op_explain "${_padded_lines}")
	string_unescape (GRDMATH_OPERATOR_EXPLAIN "${_op_explain}" NOESCAPE_SEMICOLON)

	configure_file (${GMT_SRC}/src/grdmath_explain.h.in grdmath_explain.h)

#	# grdmath_man.i
#	list_regex_replace (
#		"^[^:]+:[ ]+([^ ]+)[ ]+([0-9]+)[ ]+([0-9]+)[ ]+(.+)[.][ *]+/.*"
#		"#BfB\\\\1#BfP\t\\\\2  \\\\3\t\\\\4"
#		_op_man ${_raw_op_descriptions}
#		MATCHES_ONLY)
#	string (REPLACE ";" "\n" _op_man "${_op_man}")
#	string_unescape (_op_man "${_op_man}" NOESCAPE_SEMICOLON)
#	set (_op_man "Choose among the following ${GRDMATH_N_OPERATORS} operators.\n"
#	"\"args\" are the number of input and output arguments.\n"
#	".TS\nl l l .\nOperator\targs\tReturns\n${_op_man}\n.TE\n")
#	file (WRITE grdmath_man.i ${_op_man})
endmacro (gen_grd_math_h)

# Get something done
if (GENERATE_COMMAND STREQUAL gen_gmt_datums_h)
	gen_gmt_datums_h ()
elseif (GENERATE_COMMAND STREQUAL gen_gmt_colors_h)
	gen_gmt_colors_h ()
elseif (GENERATE_COMMAND STREQUAL gen_ps_font_info)
	gen_ps_font_info ()
elseif (GENERATE_COMMAND STREQUAL gen_gmt_ellipsoids)
	gen_gmt_ellipsoids ()
elseif (GENERATE_COMMAND STREQUAL gen_gmt_keywords_h)
	gen_gmt_keywords_h ()
elseif (GENERATE_COMMAND STREQUAL gen_gmt_dimensions_h)
	gen_gmt_dimensions_h ()
elseif (GENERATE_COMMAND STREQUAL gen_gmt_math_h)
	gen_gmt_math_h ()
elseif (GENERATE_COMMAND STREQUAL gen_grd_math_h)
	gen_grd_math_h ()
elseif (DEFINED GENERATE_COMMAND)
	message (SEND_ERROR "Unknown command: ${GENERATE_COMMAND}")
endif (GENERATE_COMMAND STREQUAL gen_gmt_datums_h)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
