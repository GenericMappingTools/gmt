#!/bin/sh
set -e

echo "*** jtests.sh: testing ncgen4 -lj"

INSTALL=/tmp/install
NETCDF4=0
KFLAG=1
STANDALONE=1

thisdir=`pwd`
if test "x$builddir" = "x"; then builddir=`pwd`/..; fi
if test "x$srcdir" = "x"; then srcdir=`dirname $0`; fi
# Make srcdir point to the netcdf-3 level */
srcdir="${srcdir}/.."

# Make buildir absolute
cd $builddir
builddir=`pwd`
# Return to home
cd $thisdir

# Make srcdir be absolute
cd $srcdir
srcdir=`pwd`
cd $builddir
# Return to home
cd $thisdir

# Locate the cdl and expected directories
cdl="${srcdir}/ncdump/cdl4"
expected="${srcdir}/ncdump/expected4"

# Locate java related stuff
JARS="${srcdir}/ncgen/ncCore-4.1.jar"
JAVA=java
JAVAC=javac

# Locate various libraries, programs, etc
NCGEN4=${builddir}/ncgen/ncgen
#NCGEN4=${builddir}/ncgen/.libs/ncgen
NCDUMP=${builddir}/ncdump/ncdump

# Define the set of tests that can be
# processed with either the -k1 or -k3 flag

TESTS3="\
test0 \
nc_enddef \
sfc_pres_temp \
simple_xy \
small \
fills \
c0 \
example_good \
nc_sync \
pres_temp_4D \
ref_tst_small \
ref_nctst \
ref_nctst_64bit_offset \
small2 \
tst_ncml \
ref_ctest1_nc4 \
ref_ctest1_nc4c \
ref_nctst_netcdf4 \
ref_nctst_netcdf4_classic"

if test "$JNI" = "1" ; then
TESTS3="$TESTS3 ref_tst_unicode ref_tst_utf8"
fi

# Define the set of tests that must be
# processed with the -k3 flag

TESTS4="\
ref_dimscope \
ref_typescope \
ref_tst_string_data \
ref_tst_comp \
ref_tst_comp2 \
ref_tst_group_data \
ref_tst_opaque_data \
ref_tst_vlen_data \
ref_tst_solar_1 \
ref_tst_solar_2 \
ref_tst_enum_data \
ref_tst_nans \
ref_tst_special_atts \
ref_const_test"

SPECIALTESTS="ref_tst_special_atts"

if test "x$JNI" = "x0" ; then
XFAILTESTS="test0 c0 ref_tst_small ref_ctest1_nc4 ref_ctest1_nc4c"
else
XFAILTESTS="ref_tst_special_atts"
fi

# Following are generally not run
# Because of the size of their output
BIGTESTS3="\
bigf1 \
bigf2 \
bigf3 \
bigr1 \
bigr2"

BIGTESTS4="ref_tst_solar_1"

# This test is both big and slow
BIGBIG3="gfs1"

if test "${KFLAG}" = 1 ; then
TESTSET="${TESTS3}"
elif test "${KFLAG}" = 3 ; then
TESTSET="${TESTS3} ${TESTS4}"
else
echo "Bad KFLAG=$KFLAG"
exit 1
fi

rm -fr results
mkdir results

failcount=0
passcount=0
xfailcount=0

for x in ${TESTSET} ; do
  echo "Testing: ${x}"
  specflag=""
  isxfail=0
  for s in $SPECIALTESTS ; do
    if test "$x" = "$s" ; then specflag="-s"; fi
  done
  for s in $XFAILTESTS ; do
    if test "$x" = "$s" ; then isxfail=1; fi
  done
  cd results
  ok=0;
  ext="cdl"
  if test -f ${cdl}/${x}.jdl ; then ext="jdl" ; fi
  if ${NCGEN4} -M${x} -lj -k${KFLAG} ${cdl}/${x}.${ext} >${x}.java ; then ok=1; else ok=0; fi
  if ${JAVAC} -d . -classpath ".:${JARS}" ${x}.java; then ok=1; else ok=0; fi
  if ${JAVA} -classpath .:${JARS} ${x} ; then ok=1; else ok=0; fi
  if ${NCDUMP} ${specflag} ${x}.nc > ${x}.dmp ; then ok=1; else ok=0; fi
  cd ..
if test 1 = 1; then
  # compare with expected
  if diff -w ${expected}/${x}.dmp results/${x}.dmp ; then
    echo "SUCCEED: ${x}"
    passcount=`expr $passcount + 1`
  elif test $isxfail = 1 ; then
    echo "XFAIL : ${x}"
    xfailcount=`expr $xfailcount + 1`
  else
    echo "FAIL: ${x}"
    failcount=`expr $failcount + 1`
  fi
fi
done

totalcount=`expr $passcount + $failcount + $xfailcount`
okcount=`expr $passcount + $xfailcount`

echo "PASSED: ${okcount}/${totalcount} ; ${xfailcount} expected failures ; ${failcount} unexpected failures"

rm -f *.class

if test $failcount -gt 0 ; then
  exit 1
else
  exit 0
fi
