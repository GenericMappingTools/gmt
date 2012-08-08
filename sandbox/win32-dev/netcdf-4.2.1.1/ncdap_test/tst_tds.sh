#!/bin/sh

# if this is part of a distcheck action, then this script
# will be executed in a different directory
# than the one containing it; so capture the path to this script
# as the location of the source directory.
srcdir=`dirname $0`
cd $srcdir
srcdir=`pwd`

# compute the build directory
builddir=`pwd`/..
# Hack for CYGWIN
if [ `uname | cut -d "_" -f 1` = "MINGW32" ]; then
    srcdir=`pwd | sed 's/\/c\//c:\//g'`
    builddir="$srcdir"/..
fi

cd ${builddir}/ncdap_test

#set -x
quiet=0
cache=1
leakcheck=0

if test "x$cache" = "x1" ; then
CACHEPARAM="[cache]"
else
CACHEPARAM=""
fi

PARAMS="[netcdf3][log]${CACHEPARAM}"

#OCLOGFILE=/dev/null
OCLOGFILE="" ; export OCLOGFILE

# Locate the testdata and expected directory
EXPECTED="${srcdir}/expecttds3"

# get the list of test files
##################################################
# Special test info
##################################################
# TDS files under 10 megabytes
# TDS Catalog: http://motherlode.ucar.edu:8080/thredds/catalog/public/dataset/catalog.html

TDSURL1="http://motherlode.ucar.edu:8080/thredds/dodsC/public/dataset"
TDSTESTS1="\
tst-striped.nc \
tst-PROFILER_RASS.nc \
tst-upc-nmm-point.nc \
testData2.nc \
testData.nc \
tst-ocean.nc \
tst-sst.nc \
tst-RUC.nc \
tst-NCEP-NAM-CONUS.nc \
tst-NCEP-SREF-CONUS.nc \
tst-NWS-NDFD-CONUS.nc \
tst-NCEP-RUC-CONUS.nc \
tst-NCEP-GFS-ALASKA.nc \
tst-PROFILER.nc"

# TDS files over 10 megabytes
TDSURL2="${TDSURL1}"
TDSTESTS2="\
tst-NCEP-DGEX-CONUS.nc \
tst-upc-nmm-grid.nc \
tst-eta.nc \
tst-NCEP-GEFS-GLOBAL.nc \
tst-GFS_Global.nc \
tst-Surface-METAR.nc"

TESTURL="${TDSURL1}"
TESTSET="${TDSTESTS1}"

# Temporarily suppress
XFAILTESTS="tst-Surface-METAR.nc"

RESULTSDIR="./results"
expected3="${srcdir}/expecttds3"

# Locate some tools
NCDUMP="${builddir}/ncdump/ncdump"
NCCOPY="${builddir}/ncdump/nccopy"

if test "x$leakcheck" = x1 ; then
VALGRIND="valgrind -q --error-exitcode=2 --leak-check=full"
else
VALGRIND=
fi

rm -fr ${RESULTSDIR}
mkdir "${RESULTSDIR}"

passcount=0
xfailcount=0
failcount=0

if test "x$quiet" = "x0" ; then 
echo "*** Testing TDS server"
echo "*** URL: ${TESTURL}"
echo "*** PARAMS: ${PARAMS}"
fi

cd ${RESULTSDIR}
for t in ${TESTSET} ; do
  name="$t"
  url="${PARAMS}${TESTURL}/$name"
  if test "x$quiet" = "x0" ; then echo "*** Testing: ${name}"; fi

  # Ignore missing cases
#  if test -f ${EXPECTED}/${name}.dmp
#  then ignore=1; else echo "Missing: ${EXPECTED}/${name}.dmp";  continue; fi

  # determine if this is an xfailtest
  isxfail=0
  for x in ${XFAILTESTS} ; do
    if test "x${name}" = "x${x}" ; then isxfail=1; fi
  done

  status=0

  if ${VALGRIND} ${NCCOPY} ${url} ${name}.nc
  then status=$status; else status=1; fi

  if ${NCDUMP} ${name}.nc > ${name}.dmp
  then status=$status; else status=1; fi

  if diff -w ${EXPECTED}/${name}.dmp ${name}.dmp
  then status=$status; else status=1; fi

  if test "x$status" = "x1" ; then
    if test "x$isxfail" = "x1" ; then status=2; fi  # xfail
  fi

  case "$status" in
  0)
    passcount=`expr $passcount + 1`
    if test "x$quiet" = "x" ; then echo "*** SUCCEED: ${name}"; fi
    ;;
  1)
    failcount=`expr $failcount + 1`
    echo "*** FAIL:  ${name}"
    ;;
  2)
    xfailcount=`expr $xfailcount + 1`
    echo "*** XFAIL : ${name}"
    ;;
  esac

done
cd ..

totalcount=`expr $passcount + $failcount + $xfailcount`
okcount=`expr $passcount + $xfailcount`

echo "*** PASSED: ${okcount}/${totalcount} ; ${xfailcount} expected failures ; ${failcount} unexpected failures"

# Ignore failures for now
#failcount=0
if test "$failcount" -gt 0 ; then
  exit 1
else
  exit 0
fi
