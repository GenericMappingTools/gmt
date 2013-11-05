#!/bin/sh -f
#--------------------------------------------------------------------
#	$Id$
#
#	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
#	See LICENSE.TXT file for copying and redistribution conditions.
#
#	This program is free software; you can redistribute it and/or
#	modify it under the terms of the GNU General Public License as
#	published by the Free Software Foundation; version 2 of the
#	License.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	Contact info: gmt.soest.hawaii.edu
#--------------------------------------------------------------------
#
# isogmt runs a GMT command or script in isolation mode.

if [ $# -eq 0 ]; then
	cat << END >&2
isogmt @GMT_PACKAGE_VERSION@ - Run GMT command or script in isolation mode

usage: isogmt <command>

	<command> is a single GMT command plus options or an executable script.
END
	exit
fi

export GMT_TMPDIR=`mktemp -d ${TMPDIR:-/tmp}/gmt.XXXXXX`
$*
rm -rf $GMT_TMPDIR
unset GMT_TMPDIR
