#!/bin/bash
#
# $Id$

# test if testsuite is up to date and setup correctly

# compare actual dirs with expected values
GMT_PATH=$(fix_mingw_path "${GMT_BINARY_DIR}/src/${CMAKE_CONFIG_TYPE:-.}/gmt5")
echo -n "gmt5 location: "
which gmt5           | grep -E "^${GMT_PATH}($|[.]exe)$" # note: depending on regexp engine the $ in the subexpression may be mandatory
echo -n "sharedir    : "
gmt5 --show-sharedir | grep "^${GMT_SHAREDIR}$"
echo -n "userdir     : "
gmtget DIR_USER     | grep "^${GMT_USERDIR}$"
echo -n "gshhg dir   : "
gmtget DIR_GSHHG    | grep "^${GSHHG_DIR}$"

# compare actual version with expected version
echo -n "gmt5 version : "
gmt5 --version       | grep "${GMT_VERSION}$"
