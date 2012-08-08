#!/bin/sh

set -e
echo ""
verbose=0

if test "x$builddir" = "x"; then builddir=`pwd`; fi
if test "x$srcdir" = "x"; then srcdir=`dirname $0`; fi

# Make buildir absolute
cd $builddir
builddir=`pwd`

# Make srcdir be absolute
cd $srcdir
srcdir=`pwd`
cd $builddir

export verbose
export srcdir
export builddir

KFLAG=1 ; export KFLAG
sh  ${srcdir}/tst_ncgen4_diff.sh
sh  ${srcdir}/tst_ncgen4_cycle.sh
exit 0


