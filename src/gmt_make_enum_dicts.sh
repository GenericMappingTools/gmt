#!/bin/bash
#
# $Id$
#
# Copyright (c) 2012-2017
# by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script just makes the include snippet gmt_enum_dict.h
# needed for GMT_API_Enum () to work.
#
cat << EOF > gmt_enum_dict.h
/*--------------------------------------------------------------------
 *      \$Id\$
 *
 *      Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *      Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

/*
 * Include file for getting GMT API enum codes programmatically via GMT_API_enum ()
 *
 * Author:      Paul Wessel
 * Date:        23-JUL-2017
 * Version:     6 API
 */

struct GMT_API_DICT *gmt_api_enums_int[] = {
EOF

egrep -v 'struct|union|enum|_GMT|define|char' gmt_resources.h | tr ',' ' ' | awk '{if (substr($1,1,4) == "GMT_") print $1, $3}' | sort -k 1 > /tmp/junk.txt
grep -v GMT_OPT_ /tmp/junk.txt | awk '{printf "\t{\"%s\", %d},\n", $1, $2}' >> gmt_enum_dict.h
cat << EOF >> gmt_enum_dict.h
	{NULL, 0}
};

struct GMT_API_DICT *gmt_api_enums_char[] = {
EOF

grep GMT_OPT_ /tmp/junk.txt | awk '{printf "\t{\"%s\", %d},\n", $1, $2}' >> gmt_enum_dict.h
cat << EOF >> gmt_enum_dict.h
	{NULL, 0}
};
EOF
rm -f /tmp/junk.txt
