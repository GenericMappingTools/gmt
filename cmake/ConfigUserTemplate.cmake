# Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# ============================================================================
# Basic setup begins here.  All settings are optional.  In most cases, setting
# CMAKE_INSTALL_PREFIX should be all you need to do in order to build GMT with
# reasonable defaults enabled. All settings are currently turned OFF. For more
# advanced settings, see ConfigUserAdvancedTemplate.cmake.
# ============================================================================

# 1.  Installation path (usually defaults to /usr/local) [auto]:
#set (CMAKE_INSTALL_PREFIX "prefix_path")

# 2.  Install convenience links for GMT modules.  Uncomment to make direct links
#     to modules so you can run "module options" without the leading "gmt" [FALSE]:
#set (GMT_INSTALL_MODULE_LINKS TRUE)

# 3a. Set full path to the GSHHG Shoreline Database directory [auto]:
#set (GSHHG_ROOT "gshhg_path")
# 3b. Copy GSHHG files to ${GMT_DATADIR}/coast [TRUE]?:
#set (COPY_GSHHG FALSE)

# 4a. Set full path to the DCW Digital Chart of the World for GMT directory [auto]:
#set (DCW_ROOT "dcw-gmt_path")
# 4b. Copy DCW files to ${GMT_DATADIR}/dcw [TRUE]?:
#set (COPY_DCW FALSE)
