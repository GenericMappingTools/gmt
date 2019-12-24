#
# Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script creates an alias for each GMT module which prepends the module
# name with "gmt". For example: alias psxy "gmt psxy"
#
# Include this file in your GMT (t)csh script or on the command line with:
#   source path/to/gmt/share/tools/gmt_aliases.csh
# If the GMT executable is not in the search path, set an extra alias:
#   alias gmt path/to/gmt

which gmt > /dev/null
if ($? == 1) then
  echo 'Error: gmt is not found in your search PATH.'
  exit 1
endif

set gmt_modules = (`gmt --show-classic`)
set compat_modules = (minmax gmtstitch gmtdp grdreformat ps2raster originator)

foreach module ( $gmt_modules $compat_modules )
  	eval 'alias $module "gmt $module"'
end
