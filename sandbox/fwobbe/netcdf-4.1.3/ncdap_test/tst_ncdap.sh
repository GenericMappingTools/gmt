#!/bin/sh

#set -x
quiet=0
leakcheck=0

OCLOGFILE=/dev/null

# Capture arguments
srcdir="$1"
builddir="$2"
mode="$3"

# Locate the testdata and expected directory
testdata3="${srcdir}/testdata3"
expected3="${srcdir}/expected3"
expected4="${srcdir}/expected4"

# get the list of test files
. ${srcdir}/tst_ncdap_shared.sh

case "$mode" in
*3)
    EXPECTED="$expected3"
    TITLE="DAP to netCDF-3 translation"
    PARAMS="[cache]"
    ;;
*4)
    EXPECTED="$expected4"
    TITLE="DAP to netCDF-4 translation"
    PARAMS="[netcdf4][cache]"
    ;;
esac

case "$mode" in
file*)
    TESTURL="$FILEURL"
    TESTSET="$FILETESTS"
    ;;
remote*)
    TESTURL="$REMOTEURL"
    TITLE="Remote $TITLE"
    TESTSET="$REMOTETESTS"
    ;;
esac

RESULTSDIR="./results"
# Locate some tools
NCDUMP="${builddir}/ncdump/ncdump"
if test "x$leakcheck" = "x1" ; then
VALGRIND="valgrind -q --error-exitcode=2 --leak-check=full"
else
VALGRIND=
fi

rm -fr ${RESULTSDIR}
mkdir "${RESULTSDIR}"

passcount=0
xfailcount=0
failcount=0

echo "*** Testing $TITLE "
echo "        Base URL: ${TESTURL}"
echo "        Client Parameters: ${PARAMS}"

cd ${RESULTSDIR}
rm -f ./.dodsrc
echo '#DODSRC' >./.dodsrc

for x in ${TESTSET} ; do
  url="${PARAMS}${TESTURL}/$x"
  if test "x$quiet" = "x0" ; then echo "*** Testing: ${x}"; fi
  # determine if this is an xfailtest
  isxfail=0
  for t in ${XFAILTESTS} ; do
    if test "x${t}" = "x${x}" ; then isxfail=1; fi
  done
  ok=1
  if ${VALGRIND} ${NCDUMP} "${url}" > ${x}.dmp ; then ok=$ok; else ok=0; fi
  # compare with expected
  if diff -w ${EXPECTED}/${x}.dmp ${x}.dmp ; then ok=$ok; else ok=0; fi
  if test "$ok" = 1 ; then
    status=0  # succeed
  elif test "x$isxfail" = "x0" ; then
    status=1  # fail
  else
    status=2  # xfail
  fi
  
  case "$status" in
  0)
    passcount=`expr $passcount + 1`
    if test "x$quiet" = "x" ; then echo "*** SUCCEED: ${x}"; fi
    ;;
  1)
    failcount=`expr $failcount + 1`
    echo "*** FAIL:  ${x}"
    ;;
  2)
    xfailcount=`expr $xfailcount + 1`
    echo "*** XFAIL: ${x}"
    ;;
  esac

done

rm -f ./.dodsrc

cd ..

totalcount=`expr $passcount + $failcount + $xfailcount`
okcount=`expr $passcount + $xfailcount`

echo "*** PASSED: ${okcount}/${totalcount} ; ${xfailcount} expected failures ; ${failcount} unexpected failures"

#failcount=0
if test "$failcount" -gt 0
then
  exit 1
else
  exit 0
fi
