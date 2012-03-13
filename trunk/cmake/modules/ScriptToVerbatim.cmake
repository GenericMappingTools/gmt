#
# $Id: CMakeLists.txt 9861 2012-03-13 14:43:02Z remko $
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

include (ManageString)

# Convert scripts to verbatim text for inclusion in TeX documentation
if (no_strip)
	grep ("[$]Id:|functions[.]sh" _file_content ${script} INVERT)
else (no_strip)
	grep ("^#|functions.sh" _file_content ${script} INVERT)
endif (no_strip)
string (REPLACE ";" "\n" _file_content "${_file_content}")
string_unescape (_verbatim_txt "${_file_content}" NOESCAPE_SEMICOLON)
file (WRITE "${txt}" "${_verbatim_txt}")

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
