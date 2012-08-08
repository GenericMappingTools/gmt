#!/bin/sh

# if this is part of a distcheck action, then this script
# will be executed in a different directory
# than the one containing it; so capture the path to this script
# as the location of the source directory.
srcdir=`dirname $0`
tmp=`echo ${srcdir}|sed -e 's/^\\\\//g'`
if test "${tmp}" = "${srcdir}" ; then
  srcdir=`pwd`/${srcdir}
  tmp=`echo ${srcdir}|sed -e 's/\\\\$//g'`
  srcdir=${tmp}
fi

thisdir=libncdap4

# compute the build directory
# Use a trick to get the proper output when using CYGWIN
# to remove e.g. c: in path
cd `pwd`/..
builddir=`pwd`
cd ${builddir}/${thisdir}

# rewrite srcdir for CYGWIN
cd ${srcdir}/..
srcdir=`pwd`
cd $builddir/${thisdir}

resultsdir="./results"

# Locate some tools
#NCDUMP=${builddir}/ncdump/ncdump
NCDUMP=./ncd

# Locate the testdata and expected directory
testdata="${srcdir}/ncdap_test/testdata3"

# Define the octest options (except -m)
OPTIONS="-h"

##################################################
# Local test info
##################################################

# Define the complete URLS

TESTURLPREFIX="file://${testdata}"

XFAILTESTS=

SYNTHETICDATA="synth1 synth2 synth3 synth4 synth5 synth6"

ACTUALDATA=\
"b31a b31 D1 Drifters \
EOSDB ingrid nestedDAS NestedSeq2 \
NestedSeq OverideExample pbug0001b SimpleDrdsExample \
test.01 test.02 test.03 test.04 \
test.05 test.06a test.06 test.07a \
test.07 test.21 test.22 \
test.23 test.31 test.32 \
test.50 test.53 test.55 \
test.56 test.57 \
test.66 test.67 test.68 test.69 \
test.an1 \
test.dfp1 test.dfr1 test.dfr2 test.dfr3 \
test.gr1 test.gr2 test.gr3 test.gr4 \
test.gr5 test.PointFile test.sds1 \
test.sds2 test.sds3 test.sds4 test.sds5 \
test.sds6 test.sds7 test.vs1 \
test.vs2 test.vs3 test.vs4 test.vs5 whoi \
test.SwathFile \
1990-S1700101.HDF.WVC_Lat 1998-6-avhrr.dat"

TESTSET="${SYNTHETICDATA} ${ACTUALDATA}"

##################################################
# Testing Procedure
##################################################

rm -fr ${resultsdir}
mkdir ${resultsdir}

rm -f ${OCLOGFILE}

passcount=0
xfailcount=0
failcount=0

for x in ${TESTSET} ; do
  url="${TESTURLPREFIX}/$x"
  echo "*** Testing: ${x}"
	# determine if this is an xfailtest
	isxfail=
	for t in ${XFAILTESTS} ; do
	if test "x${t}" = "x${x}" ; then isxfail=1; fi
	done
  ${NCDUMP} ${OPTIONS} ${url} > ${resultsdir}/${x}.dmp
  # compare with expected
if test 0 = 1 ; then
  if diff -w ${expected}/${x}.dmp ${resultsdir}/${x}.dmp ; then
    echo "*** SUCCEED: ${x}"
    passcount=`expr $passcount + 1`
  elif test "x${isxfail}" = "x1" ; then
    echo "*** XFAIL : ${x}"
    xfailcount=`expr $xfailcount + 1`
  else
    echo "*** FAIL: ${x}"
    failcount=`expr $failcount + 1`
  fi
fi
done

totalcount=`expr $passcount + $failcount + $xfailcount`
okcount=`expr $passcount + $xfailcount`

echo "*** PASSED: ${okcount}/${totalcount} ; ${xfailcount} expected failures ; ${failcount} unexpected failures"

if test "$failcount" -gt 0 ; then
  exit 1
else
  exit 0
fi
