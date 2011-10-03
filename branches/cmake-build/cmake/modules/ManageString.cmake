#
# $Id$
#
# - Collection of String utility macros.
# Defines the following macros:
#   STRING_ESCAPE(var str [NOESCAPE_SEMICOLON] [ESCAPE_VARIABLE])
#   - Encode characters that would be expanded '\', ';', '$', '#'
#     * Parameters:
#       + var: A variable that stores the result.
#       + str: The NAME of a variable that holds the string.
#       + NOESCAPE_SEMICOLON: (Optional) Do not escape semicolons.
#       + ESCAPE_VARIABLE: (Optional) Also escape '$'
#
#   STRING_UNESCAPE(var str [NOESCAPE_SEMICOLON] [ESCAPE_VARIABLE])
#   - Decode '\', ';', '$', '#', the reverse of STRING_ESCAPE
#     * Parameters:
#       + var: A variable that stores the result.
#       + str: The encoded string
#       + NOESCAPE_SEMICOLON: (Optional) Do not decode semicolons.
#       + ESCAPE_VARIABLE: (Optional) Also decode '$'
#
#   STRING_UNQUOTE(var str)
#   - Remove double quote marks and quote marks around a string.
#     * Parameters:
#       + var: A variable that stores the result.
#       + str: The NAME of a variable that holds the string.
#
#   STRING_JOIN(var delimiter str_list [str...])
#   - Concatenate strings, with delimiter inserted between strings.
#     * Parameters:
#       + var: A variable that stores the result.
#       + str_list: A list of string.
#       + str: (Optional) more string to be join.
#
#   STRING_SPLIT(var delimiter str [NOESCAPE_SEMICOLON] [ESCAPE_VARIABLE] [NOENCODE])
#   - Split a string into a list using a delimiter, which can be in 1 or more
#     characters long.
#     * Parameters:
#       + var: A variable that stores the result.
#       + delimiter: The NAME of a variable that holds the delimiter.
#       + str: The NAME of a variable that holds the string to split.
#       + NOESCAPE_SEMICOLON: (Optional) Do not escape semicolons.
#       + NOENCODE: (Optional) Do not encode/decode string
#
#   FILE2LIST (var filename [filename...])
#   - Read a file to list, escape '#', '\' and ';'
#     * Parameters:
#       + var: A variable that stores the list.
#       + filename: Files to read
#
#   GREP (pattern var filename [filename...] [LITERALLY] [INVERT])
#   - Read a file to list, escape '#', '\' and ';'
#     * Parameters:
#       + pattern: Regex to match
#       + var: A variable that stores the matching lines.
#       + filename: Files to read
#       + LITERALLY: (Optional) Match the pattern string literally
#       + INVERT: (Optional) Invert match
#
#   LIST_REGEX_REPLACE (<regular_expression> <replace_expression>
#                       <output list> <list> [<list>...])
#
#   LIST_REGEX_GET (<regular_expression> <output list>
#                   <list> [<list>...] [INVERT])
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

#if(NOT DEFINED _MANAGE_STRING_CMAKE_)
	set(_MANAGE_STRING_CMAKE_ "DEFINED")

	# STRING_ESCAPE(var str [NOESCAPE_SEMICOLON] [ESCAPE_VARIABLE])
	macro(STRING_ESCAPE var str)
		# ';' and '\' are tricky, need to be encoded.
		# '#' => '#H'
		# '$' => '#D'
		# ';' => '#S'
		# '\' => '#B'
		# '|' => '#P'
		set(_ESCAPE_VARIABLE)
		set(_NOESCAPE_SEMICOLON)
		string(REPLACE "#" "#H" _ret "${${str}}")
		string(REPLACE "\\" "#B" _ret "${_ret}")
		#string(REPLACE "|" "#P" _ret "${_ret}")
		foreach(_arg ${ARGN})
			if(${_arg} STREQUAL "NOESCAPE_SEMICOLON")
				set(_NOESCAPE_SEMICOLON "NOESCAPE_SEMICOLON")
			elseif(${_arg} STREQUAL "ESCAPE_VARIABLE")
				set(_ESCAPE_VARIABLE "ESCAPE_VARIABLE")
				string(REPLACE "$" "#D" _ret "${_ret}")
			endif(${_arg} STREQUAL "NOESCAPE_SEMICOLON")
		endforeach(_arg)
		if(NOT _NOESCAPE_SEMICOLON)
			string(REPLACE ";" "#S" _ret "${_ret}")
		endif(NOT _NOESCAPE_SEMICOLON)
		set(${var} "${_ret}")
	endmacro(STRING_ESCAPE var str)

	# STRING_UNESCAPE(var str [NOESCAPE_SEMICOLON] [ESCAPE_VARIABLE])
	macro(STRING_UNESCAPE var str)
		# '#B' => '\'
		# '#D' => '$'
		# '#H' => '#'
		# '#P' => '|'
		# '#S' => ';'
		set(_ESCAPE_VARIABLE)
		set(_NOESCAPE_SEMICOLON)
		set(_ret "${str}")
		foreach(_arg ${ARGN})
			if(${_arg} STREQUAL "NOESCAPE_SEMICOLON")
				set(_NOESCAPE_SEMICOLON "NOESCAPE_SEMICOLON")
			elseif(${_arg} STREQUAL "ESCAPE_VARIABLE")
				set(_ESCAPE_VARIABLE "ESCAPE_VARIABLE")
				string(REPLACE "#D" "$" _ret "${_ret}")
			endif(${_arg} STREQUAL "NOESCAPE_SEMICOLON")
		endforeach(_arg)

		if(_NOESCAPE_SEMICOLON)
			# ';' => '#s'
			string(REPLACE "#S" ";" _ret "${_ret}")
		else(_NOESCAPE_SEMICOLON)
			string(REPLACE "#S" "\\;" _ret "${_ret}")
		endif(_NOESCAPE_SEMICOLON)

		if(_ESCAPE_VARIABLE)
			# '#d' => '$'
			string(REPLACE "#D" "$" _ret "${_ret}")
		endif(_ESCAPE_VARIABLE)
		#string(REPLACE "#P" "|" _ret "${_ret}")
		string(REPLACE "#B" "\\" _ret "${_ret}")
		string(REPLACE "#H" "#" _ret "${_ret}")
		set(${var} "${_ret}")
	endmacro(STRING_UNESCAPE var str)

	# STRING_UNQUOTE(var str)
	macro(STRING_UNQUOTE var str)
		# escape special chars
		string_escape(_str ${str} NOESCAPE_SEMICOLON)

		if(_str MATCHES "^[ \t\r\n]+")
			string(REGEX REPLACE "^[ \t\r\n]+" "" _str "${_str}")
		endif(_str MATCHES "^[ \t\r\n]+")
		if(_str MATCHES "^\"")
			# double quote
			string(REGEX REPLACE "\"\(.*\)\"[ \t\r\n]*$" "\\1" _str "${_str}")
		elseif(_str MATCHES "^'")
			# single quote
			string(REGEX REPLACE "'\(.*\)'[ \t\r\n]*$" "\\1" _str "${_str}")
		else(_str MATCHES "^\"")
			set(_str)
		endif(_str MATCHES "^\"")

		# unencoding
		string_unescape(${var} "${_str}" NOESCAPE_SEMICOLON)
	endmacro(STRING_UNQUOTE _var _str)

  # STRING_JOIN(var delimiter str_list [str...])
	macro(STRING_JOIN var delimiter str_list)
		set(_ret)
		foreach(_str ${str_list})
			if(_ret)
				set(_ret "${_ret}${delimiter}${_str}")
			else(_ret)
				set(_ret "${_str}")
			endif(_ret)
		endforeach(_str ${str_list})

		foreach(_str ${ARGN})
			if(_ret)
				set(_ret "${_ret}${delimiter}${_str}")
			else(_ret)
				set(_ret "${_str}")
			endif(_ret)
		endforeach(_str ${str_list})
		set(${var} "${_ret}")
	endmacro(STRING_JOIN var delimiter str_list)

	# STRING_SPLIT(var delimiter str [NOESCAPE_SEMICOLON] [ESCAPE_VARIABLE] [NOENCODE])
	macro(STRING_SPLIT var delimiter str)
		set(_max_tokens)
		set(_NOESCAPE_SEMICOLON)
		set(_ESCAPE_VARIABLE)
		set(_NOENCODE)
		foreach(_arg ${ARGN})
			if(${_arg} STREQUAL "NOESCAPE_SEMICOLON")
				set(_NOESCAPE_SEMICOLON "NOESCAPE_SEMICOLON")
			elseif(${_arg} STREQUAL "ESCAPE_VARIABLE")
				set(_ESCAPE_VARIABLE "ESCAPE_VARIABLE")
			elseif(${_arg} STREQUAL "NOENCODE")
				set(_NOENCODE "NOENCODE")
			else(${_arg} STREQUAL "NOESCAPE_SEMICOLON")
				set(_max_tokens ${_arg})
			endif(${_arg} STREQUAL "NOESCAPE_SEMICOLON")
		endforeach(_arg)

		if(NOT _max_tokens)
			set(_max_tokens -1)
		endif(NOT _max_tokens)

		if(_NOENCODE)
			set(_str ${${str}})
			set(_delimiter ${${delimiter}})
		else(_NOENCODE)
			string_escape(_str ${str} ${_NOESCAPE_SEMICOLON} ${_ESCAPE_VARIABLE})
			string_escape(_delimiter ${delimiter} ${_NOESCAPE_SEMICOLON} ${_ESCAPE_VARIABLE})
		endif(_NOENCODE)

		set(_str_list)
		set(_token_count 0)
		string(LENGTH "${_delimiter}" _de_len)

		while(NOT _token_count EQUAL _max_tokens)
			math(EXPR _token_count ${_token_count}+1)
			if(_token_count EQUAL _max_tokens)
				# last token, no need splitting
				set(_str_list ${_str_list} "${_str}")
			else(_token_count EQUAL _max_tokens)
				# in case encoded characters are delimiters
				string(LENGTH "${_str}" _str_len)
				set(_index 0)
				set(_token)
				set(_str_remain)
				math(EXPR _str_end ${_str_len}-${_de_len}+1)
				set(_bound "k")
				while(_index LESS _str_end)
					string(SUBSTRING "${_str}" ${_index} ${_de_len} _str_cursor)
					if(_str_cursor STREQUAL _delimiter)
						# get the token
						string(SUBSTRING "${_str}" 0 ${_index} _token)
						# get the rest
						math(EXPR _rest_index ${_index}+${_de_len})
						math(EXPR _rest_len ${_str_len}-${_index}-${_de_len})
						string(SUBSTRING "${_str}" ${_rest_index} ${_rest_len} _str_remain)
						set(_index ${_str_end})
					else(_str_cursor STREQUAL _delimiter)
						math(EXPR _index ${_index}+1)
					endif(_str_cursor STREQUAL _delimiter)
				endwhile(_index LESS _str_end)

				if(_str_remain)
					list(APPEND _str_list "${_token}")
					set(_str "${_str_remain}")
				else(_str_remain)
					# meaning: end of string
					list(APPEND _str_list "${_str}")
					set(_max_tokens ${_token_count})
				endif(_str_remain)
			endif(_token_count EQUAL _max_tokens)
		endwhile(NOT _token_count EQUAL _max_tokens)

		if(_NOENCODE)
			set(${var} "${_str_list}")
		else(_NOENCODE)
			# unencoding
			string_unescape(${var} "${_str_list}" ${_NOESCAPE_SEMICOLON} ${_ESCAPE_VARIABLE})
		endif(_NOENCODE)
	endmacro(STRING_SPLIT var delimiter str)

	# FILE2LIST (var filename [filename...])
	macro (FILE2LIST _OUT _FILE)
		set(${_OUT}) # clear
		if (NOT FILESIZE_LIMIT)
			# default: read max 512KiB
			set(FILESIZE_LIMIT 524288)
		endif (NOT FILESIZE_LIMIT)
		foreach (_file ${_FILE} ${ARGN})
			file (READ ${_file} _file_content LIMIT ${FILESIZE_LIMIT})
			string_escape (_file_content _file_content) # ; -> #S and \ -> #B
			string (REPLACE "\n" ";" _file_content ${_file_content})
			list(APPEND ${_OUT} ${_file_content})
		endforeach (_file ${_FILE} ${ARGN})
	endmacro (FILE2LIST _OUT _FILE)

	# HEAD (string): print at most 80 chars of a string
	macro (HEAD _STR)
		string(LENGTH "${_STR}" _len)
		if (_len GREATER 80)
			set(_len 80)
		endif ()
		string(SUBSTRING "${_STR}" 0 ${_len} _prn)
		message("H> ${_prn}")
	endmacro (HEAD _STR)

	# GREP (pattern var filename [filename...] [LITERALLY] [INVERT])
	macro (GREP _PATTERN _OUT _FILE)
		set (_args ${ARGN})
		list (FIND _args "LITERALLY" _LITERALLY)
		if (_LITERALLY EQUAL -1)
			set(_LITERALLY)
		else(_LITERALLY EQUAL -1)
			set(_LITERALLY TRUE)
			list(REMOVE_ITEM _args "LITERALLY")
		endif(_LITERALLY EQUAL -1)
		list (FIND _args "INVERT" _INVERT)
		if (_INVERT EQUAL -1)
			set(_INVERT)
		else(_INVERT EQUAL -1)
			set(_INVERT TRUE)
			list(REMOVE_ITEM _args "INVERT")
		endif(_INVERT EQUAL -1)

		set(_matches)
		foreach (_file ${_FILE} ${_args})
			file2list (_list ${_file})
			foreach (_line ${_list})
				#message("i> ${_line}")
				set(_ismatching)
				if (_LITERALLY)
					string(FIND ${_line} ${_PATTERN} _position)
					if (_position GREATER 0)
						set(_ismatching TRUE)
					endif ()
				else (_LITERALLY)
					string(REGEX MATCH ${_PATTERN} _ismatching ${_line})
				endif (_LITERALLY)
				if ((_ismatching AND NOT _INVERT) OR (NOT _ismatching AND _INVERT))
					list (APPEND _matches ${_line})
				endif ()
				#message("f> ${_file}")
				#head(${_line})
			endforeach (_line)
		endforeach (_file)
		set (${_OUT} ${_matches})
		#message("p: ${_PATTERN}")
	endmacro(GREP _PATTERN _OUT _FILE)

	# LIST_REGEX_REPLACE (<regular_expression> <replace_expression> <output list> <list> [<list>...])
	# Note: have to double all escapes!
	macro (LIST_REGEX_REPLACE _REGEX _REP _OUT _LIST)
		#message("pattern: \"${_REGEX}\"")
		set (_args ${ARGN})
		list (FIND _args "MATCHES_ONLY" _MATCHES_ONLY)
		if (_MATCHES_ONLY EQUAL -1)
			set(_MATCHES_ONLY)
		else(_MATCHES_ONLY EQUAL -1)
			set(_MATCHES_ONLY TRUE)
			list(REMOVE_ITEM _args "MATCHES_ONLY")
		endif(_MATCHES_ONLY EQUAL -1)
		list (FIND _args "STRIP" _STRIP)
		if (_STRIP EQUAL -1)
			set(_STRIP)
		else(_STRIP EQUAL -1)
			set(_STRIP TRUE)
			list(REMOVE_ITEM _args "STRIP")
		endif(_STRIP EQUAL -1)

		set(${_OUT})
		foreach (_line ${_LIST} ${_args})
			string (REGEX REPLACE
				"${_REGEX}"
				"${_REP}" _replacement ${_line})
			if (_MATCHES_ONLY AND (_replacement STREQUAL _line))
				set(_replacement)
			endif ()
			if (_STRIP)
				string (STRIP "${_replacement}" _replacement)
			endif (_STRIP)
			list (APPEND ${_OUT} ${_replacement})
			#head("l: ${_line}")
			#head("r: ${_replacement}")
		endforeach (_line ${_LIST} ${_args})
	endmacro (LIST_REGEX_REPLACE _REGEX _REP _OUT _LIST)

	# LIST_REGEX_GET (<regular_expression> <output list> <list> [<list>...])
	# Note: have to double all escapes!
	macro (LIST_REGEX_GET _REGEX _OUT _LIST)
		#message("pattern: ${_REGEX}")
		set (_args ${ARGN})
		list (FIND _args "INVERT" _INVERT)
		if (_INVERT EQUAL -1)
			set(_INVERT)
		else(_INVERT EQUAL -1)
			set(_INVERT TRUE)
			list(REMOVE_ITEM _args "INVERT")
		endif(_INVERT EQUAL -1)

		set(${_OUT})
		foreach (_line ${_LIST} ${_args})
			set(_ismatching)
			string(REGEX MATCH ${_REGEX} _ismatching ${_line})
			if ((_ismatching AND NOT _INVERT) OR (NOT _ismatching AND _INVERT))
				list (APPEND ${_OUT} ${_line})
			endif ()
		endforeach (_line ${_LIST} ${_args})
	endmacro (LIST_REGEX_GET _REGEX _OUT _LIST)

	# STRING_PAD (string length): pad a string with spaces to the given length
	macro (STRING_PAD _STR _LEN)
		set (_args ${ARGN})
		list (FIND _args "RIGHT" _RIGHT)
		if (_RIGHT EQUAL -1)
			set (_RIGHT)
		else (_RIGHT EQUAL -1)
			set (_RIGHT TRUE)
			list (REMOVE_ITEM _args "RIGHT")
		endif (_RIGHT EQUAL -1)

		# get maker string, split string at marker,
		# and make woring copy
		list (LENGTH _args _padding)
		if (_padding)
			list(GET _args 0 _padding)
			string_split (_split _padding ${_STR}
				NOESCAPE_SEMICOLON
				NOENCODE)
			if (_RIGHT)
				# need to right align 2nd part
				list (GET _split 1 _copy)
			else (_RIGHT)
				# left align 1st part
				list (GET _split 0 _copy)
			endif (_RIGHT)
		else (_padding)
			# no marker: copy complete string
			set (_copy ${${_STR}})
		endif (_padding)

		string (LENGTH ${_copy} _orig_len)
		if (_orig_len LESS ${_LEN})
			set (_big_space "                                                       ")
			if (_RIGHT)
				# right align: prepend space
				math (EXPR _extra_len "${_LEN} - ${_orig_len}")
				string (SUBSTRING ${_big_space} 0 ${_extra_len} _extra_space)
				set (_copy "${_extra_space}${_copy}")
			else (_RIGHT)
				# left align: append space
				set (_big_str "${_copy}${_big_space}")
				string (SUBSTRING ${_big_str} 0 ${_LEN} _copy)
			endif (_RIGHT)
		endif (_orig_len LESS ${_LEN})

		# join split string
		if (_padding)
			if (_RIGHT)
				# join copy w 1st part
				list (GET _split 0 _part)
				set (_copy "${_part}${_copy}")
			else (_RIGHT)
				# join copy w 2nd part
				list (GET _split 1 _part)
				set (_copy "${_copy}${_part}")
			endif (_RIGHT)
		endif (_padding)

		# replace string with working copy
		set (${_STR} ${_copy})
	endmacro (STRING_PAD _STR _LEN)

	#endif(NOT DEFINED _MANAGE_STRING_CMAKE_)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
