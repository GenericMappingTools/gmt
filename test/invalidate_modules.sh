#--------------------------------------------------------------------
#
#  Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
#  See LICENSE.TXT file for copying and redistribution conditions.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation; version 3 or any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  Contact info: www.generic-mapping-tools.org
#--------------------------------------------------------------------*/
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
