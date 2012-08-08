#!/bin/sh
#set -e
#set -x
echo "*** ctests.sh: testing ncgen4 -c"

KFLAG=3

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


# get some config.h parameters
if test -f ${builddir}/config.h ; then
  if fgrep -e '#define USE_NETCDF4 1' ${builddir}/config.h >/dev/null ; then
    NETCDF4=1
  else
    NETCDF4=0
  fi    
  if fgrep -e '#define USE_DAP 1' ${builddir}/config.h >/dev/null ; then
    DAP=1
  else
    DAP=0
  fi    


else
  echo "Cannot locate config.h"
  exit 1
fi

# Locate the cdl and expected directories
cdl="${srcdir}/ncdump/cdl4"
expected="${srcdir}/ncdump/expected4"

# Locate various libraries, programs, etc
# The hard part is locating the HDF5 and Z libraries

NCGEN4=${builddir}/ncgen4/ncgen4
#NCGEN4=${builddir}/ncgen4/.libs/ncgen4
NCDUMP=${builddir}/ncdump/ncdump

HDFPATH=""
ZPATH=""
SZPATH=""
CURLPATH=""
CC=cc
if test -f ${builddir}/Makefile ; then
  if test "x$NETCDF4" = "x1"; then
    tmp=`sed -e '/^HDF5DIR/p' -e d <${builddir}/Makefile|tr -d " "`
    HDFPATH=`echo $tmp |cut -d '=' -f2`
    tmp=`sed -e '/^ZLIBDIR/p' -e d <${builddir}/Makefile|tr -d " "`
    ZPATH=`echo $tmp |cut -d '=' -f2`
    tmp=`sed -e '/^SZLIBDIR/p' -e d <${builddir}/Makefile|tr -d " "`
    SZPATH=`echo $tmp |cut -d '=' -f2`
  fi
  if test "x$DAP" = "x1"; then
    tmp=`sed -e '/^CURLDIR/p' -e d <${builddir}/Makefile|tr -d " "`
    CURLPATH=`echo $tmp |cut -d '=' -f2`
  fi
  tmp=`sed -e "/^CC\ /p" -e d <${builddir}/Makefile|tr -d " "`
  CC=`echo $tmp |cut -d '=' -f2`
else
  echo "No Makefile => cannot locate HDF5 path"
  exit 1
fi

if test "x$ZPATH" = "x" ; then
  ZPATH="$SZPATH"
fi
if test "x$ZPATH" = "x" ; then
  echo "NO libz or libsz specified"
  exit 1
fi

INCL=""
LIBLOCS=""
RPATH=""

if test "x$NETCDF4" = "x1"; then
NCLIBPATH="${builddir}/libsrc4/.libs"
NCINCLPATH="${srcdir}/libsrc4"
else
NCPATH="${builddir}/libsrc/.libs"
NCINCLPATH="${srcdir}/libsrc"
fi

INCL="-I ${NCINCLPATH}"

if test "x$NETCDF4" = "x1"; then
LIBLOCS="-L ${NCLIBPATH} -L ${HDFPATH}/lib -L ${ZPATH}/lib"
LIBS="${LIBLOCS} -lnetcdf -lhdf5_hl -lhdf5 -lz -lm"
#RPATH=-Wl,-rpath,${NCLIBPATH},-rpath,${HDFPATH}/lib,-rpath,${ZPATH}/lib
else
LIBLOCS="-L ${NCLIBPATH}"
LIBS="${LIBLOCS} -lnetcdf -lm"
#RPATH="-Wl,-rpath,${NCLIBPATH}"
fi

if test "x$DAP" = "x1"; then
LIBS="${LIBS} -L ${CURLPATH}/lib -lcurl"
#RPATH="${RPATH},-rpath,${CURLPATH}/lib"
fi

# Define the set of tests that can be
# processed with either the -k1 or -k3 flag

TESTS3="\
test0 \
nc_enddef \
ref_tst_unicode \
ref_tst_utf8 \
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

XFAILTESTS="ref_tst_special_atts"

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

set -e

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
  ok=0;
  cd results
  if ${NCGEN4} -lc -k${KFLAG} ${cdl}/${x}.cdl >${x}.c
    then ok=1; else ok=0; fi
  if $CC ${INCL} -c ${x}.c
    then ok=1; else ok=0; fi
  if $CC -o ${x}.exe ${x}.o ${LIBS} ${RPATH}
    then ok=1; else ok=0; fi
  if ./${x}.exe
    then ok=1; else ok=0; fi
  if ${NCDUMP} ${specflag} ${x}.nc > ${x}.dmp  
    then ok=1; else ok=0; fi
  cd ..
if test 1 = 1; then
  # compare with expected
  if diff -b -w ${expected}/${x}.dmp results/${x}.dmp
    then ok=1; else ok=0; fi
  if test "$ok" = "1" ; then
    echo "*** PASS: ${x}"
    passcount=`expr $passcount + 1`
  elif test $isxfail = 1 ; then
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

echo "PASSED: ${okcount}/${totalcount} ; ${xfailcount} expected failures ; ${failcount} unexpected failures"

#if test $failcount -gt 0 ; then
#  exit 1
#else
  exit 0
#fi
