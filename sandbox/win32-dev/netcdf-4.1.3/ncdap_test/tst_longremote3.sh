#!/bin/sh

# if this is part of a distcheck action, then this script
# will be executed in a different directory
# than the one containing it; so capture the path to this script
# as the location of the source directory.
srcdir=`dirname $0`

# compute the build directory
# Do a hack to remove e.g. c: for CYGWIN
cd `pwd`
builddir=`pwd`/..
# Hack for CYGWIN
cd $srcdir
srcdir=`pwd`
cd ${builddir}/ncdap_test

sh ${srcdir}/tst_remote.sh "$srcdir" "$builddir" "3" "" "long"
exit
