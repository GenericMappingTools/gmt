#!/bin/bash
#
# $Id$

# test if testsuite is up to date and setup correctly

# compare actual dirs with expected values
GMT_PATH=$(fix_mingw_path "${GMT_BINARY_DIR}/src/${CMAKE_CONFIG_TYPE:-.}/gmt")
echo -n "gmt location: "
which gmt           | grep -E "^${GMT_PATH}(|[.]exe)$"
echo -n "sharedir    : "
gmt --show-sharedir | grep "^${GMT_SHAREDIR}$"
echo -n "userdir     : "
gmtget DIR_USER     | grep "^${GMT_USERDIR}$"
echo -n "gshhg dir   : "
gmtget DIR_GSHHG    | grep "^${GSHHG_DIR}$"

# comapre actual version with expected version
echo -n "gmt version : "
gmt --version       | grep "${GMT_VERSION}$"
