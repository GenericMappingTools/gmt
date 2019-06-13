#!/usr/bin/env bash
#
# Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script creates a function for each GMT module which calls the module
# via the main GMT executable (gmt <module>).
#
# Include this file in your GMT bash script or on the command line with:
#   source $(gmt --show-datadir)/tools/gmt_functions.sh
# If the GMT executable is not in the search path, set an extra function:
#   function gmt () { path/to/gmt "$@"; }
#   export -f gmt

# check for bash
[ -z "$BASH_VERSION" ] && return

gmt_modules=`gmt --show-modules`
compat_modules="minmax gmt2rgb gmtstitch gmtdp grdreformat ps2raster"

for module in ${gmt_modules} ${compat_modules}; do
	eval "function ${module} () { gmt ${module} \"\$@\"; }"
 	eval "export -f ${module}"
done
