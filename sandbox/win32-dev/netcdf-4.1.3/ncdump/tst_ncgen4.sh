#!/bin/sh
verbose=1
set -e

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
echo "Performing diff tests: k=1"
sh ${srcdir}/tst_ncgen4_diff.sh
echo "Performing cycle tests: k=1"
sh  ${srcdir}/tst_ncgen4_cycle.sh
KFLAG=3 ; export KFLAG
echo "Performing diff tests: k=3"
sh  ${srcdir}/tst_ncgen4_diff.sh
echo "Performing cycle tests: k=3"
sh  ${srcdir}/tst_ncgen4_cycle.sh
KFLAG=4 ; export KFLAG
echo "Performing diff tests: k=4"
sh  ${srcdir}/tst_ncgen4_diff.sh
echo "Performing cycle tests: k=4"
sh  ${srcdir}/tst_ncgen4_cycle.sh
exit

