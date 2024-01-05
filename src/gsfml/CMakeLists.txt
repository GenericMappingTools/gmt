#
# Copyright (c) 1991-2024 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; version 3 or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# Contact info: www.generic-mapping-tools.org
#-------------------------------------------------------------------------------
#
# CMake settings for supplemental package:
#
#   SUPPL_NAME:          name of the supplemental package
#   SUPPL_HEADERS:       header files. Will be installed if BUILD_DEVELOPER is TRUE
#   SUPPL_PROGS_SRCS:    list of C source codes for supplemental modules
#   SUPPL_LIB_SRCS:      list of C source codes for supplemental library
#   SUPPL_EXAMPLE_FILES: README and other example files
#   SUPPL_EXAMPLE_PROGS: Example scripts
#

set (SUPPL_NAME gsfml)
set (SUPPL_HEADERS CK1995n.h Chron_Normal.h  Chron_Reverse.h  GST2004n.h GST2012n.h Geek2007n.h
                   CK1995r.h Chron_Normal2.h Chron_Reverse2.h GST2004r.h GST2012r.h Geek2007r.h
                   fz_analysis.h)
AUX_SOURCE_DIRECTORY (longopt SUPPL_LONG_OPT_H) 
set (SUPPL_PROGS_SRCS fzanalyzer.c fzblender.c mlconverter.c ${SUPPL_LONG_OPT_H})
set (SUPPL_LIB_SRCS ${SUPPL_PROGS_SRCS})
set (SUPPL_EXAMPLE_FILES README.gsfml)

install (PROGRAMS fz_funcs.sh
    RENAME fz_funcs${GMT_INSTALL_NAME_SUFFIX}.sh
    DESTINATION ${GMT_BINDIR}
    COMPONENT Runtime)
install (PROGRAMS fzinformer
    RENAME fz_funcs${GMT_INSTALL_NAME_SUFFIX}
    DESTINATION ${GMT_BINDIR}
    COMPONENT Runtime)
install (PROGRAMS fzmapper
    RENAME fz_funcs${GMT_INSTALL_NAME_SUFFIX}
    DESTINATION ${GMT_BINDIR}
    COMPONENT Runtime)
install (PROGRAMS fzmodeler
    RENAME fzmodeler${GMT_INSTALL_NAME_SUFFIX}
    DESTINATION ${GMT_BINDIR}
    COMPONENT Runtime)
install (PROGRAMS fzprofiler
    RENAME fzprofiler${GMT_INSTALL_NAME_SUFFIX}
    DESTINATION ${GMT_BINDIR}
    COMPONENT Runtime)
