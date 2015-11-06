#!/bin/bash
#
# $Id$

# test if testsuite is up to date and setup correctly
# make sure we are using the gmt executable from the build tree

# compare actual dirs with expected values
echo -n "gmt location: "
gmt --show-bindir       | grep "^${GMT_BINARY_DIR}/src$"
echo -n "sharedir    : "
gmt --show-sharedir     | grep "^${GMT_SHAREDIR}$"
echo -n "datadir     : "
gmt --show-datadir      | grep "^${GMT_DATADIR}$"
echo -n "gmt gshhg dir   : "
gmt gmtget DIR_GSHHG    | grep "^${GSHHG_DIR}$"
echo -n "gmt DCW dir     : "
gmt gmtget DIR_DCW      | grep "^${DCW_DIR}$"

# compare actual version with expected version
echo -n "gmt version : "
gmt --version           | grep "${GMT_VERSION}$"
