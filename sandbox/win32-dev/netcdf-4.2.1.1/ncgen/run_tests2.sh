#!/bin/sh
# This shell script runs the ncdump tests.
# $Id$

echo "*** creating test0.nc from test0.cdl..."
if !(../ncdump/ncdump -n c1 $(srcdir)/ctest0.nc > ctest1.cdl); then
    echo "ncgen test failed! Sorry!" && exit 1
fi

echo "*** Tests successful!"
exit 0
