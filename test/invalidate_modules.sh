#
# Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This script creates a function for each GMT module which invalidates
# direct module call.  This is used to make sure test scripts properly
# prefix modules with "gmt", which is needed to make sure that the
# shell does not execute modules from PATH but instead uses the gmt
# executable from the build dir.
#

gmt_modules=$(gmt --show-classic)

for module in ${gmt_modules}; do
  eval "function ${module} () { false; }"
  eval "export -f ${module}"
done
