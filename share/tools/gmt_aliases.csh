# $Id$
#
# Copyright (c) 1991-2019 by P. Wessel, W. H. F. Smith, R. Scharroo,
# J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script creates an alias for each GMT module which prepends the module
# name with "gmt". For example: alias psxy "gmt psxy"
#
# Include this file in your GMT (t)csh script or on the command line with:
#   source path/to/gmt/share/tools/gmt_aliases.sh
# If the GMT executable is not in the search path, set an extra alias:
#   alias gmt path/to/gmt

set gmt_modules = (`gmt --show-modules`)
set compat_modules = (minmax gmt2rgb gmtstitch gmtdp grdreformat ps2raster)

foreach module ( $gmt_modules $compat_modules )
  	eval 'alias $module "gmt $module"'
end
