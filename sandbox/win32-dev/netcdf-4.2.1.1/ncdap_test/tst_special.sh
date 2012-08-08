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

#EXTSET="hdr dmp"
EXTSET="dmp"

#set -x
quiet=0
leakcheck=0

# Pick one
CACHE=""
#CACHE="[cache]"
PARAMS="[netcdf3][log]${CACHE}"

#OCLOGFILE=/dev/null
OCLOGFILE="" ; export OCLOGFILE

# Locate the testdata and expected directory
EXPECTED="${srcdir}/special3"

# get the list of test files
##################################################
# Special test info
##################################################
SPECIALTESTS="\
O2_ann_nobs_1deg.nc;http://ferret.pmel.noaa.gov/thredds/dodsC/data/PMEL/WOA01/english/annual/O2_ann_nobs_1deg.nc \
500m;http://ferret.pmel.noaa.gov/thredds/dodsC/hfrnet/agg/500m \
qscat_high_wind;http://apdrc.soest.hawaii.edu/dods/public_data/satellite_product/QSCAT/qscat_high_wind \
TOMS-EP_L3-TOMSEPL3_1997m0107_v8.HDF;http://disc1.sci.gsfc.nasa.gov/opendap/Earth_Probe_TOMS_Level3/TOMSEPL3/1997/TOMS-EP_L3-TOMSEPL3_1997m0107_v8.HDF \
ssta.dods;http://iridl.ldeo.columbia.edu/SOURCES/.ENSOFORECAST/.LDEO2/.12MON/.ssta/dods \
dods;http://ingrid.ldeo.columbia.edu/SOURCES/.OBERHUBER/dods \
d055.nc;http://ilikai.soest.hawaii.edu/cgi-bin/nph-dods/fast/d055.nc \
n15.20091003.2145.usf.sst.hdf;http://www.imars.usf.edu/dods-bin/nph-dods/SST_NEW/husf/products/hdf/2009.10/n15.20091003.2145.usf.sst.hdf \
totalAagg;http://nomads.ncdc.noaa.gov/thredds/dodsC/oisst2/totalAagg \
world-unfilter-monthly.nc;http://dapper.pmel.noaa.gov/dapper/oscar/world-unfilter-monthly.nc \
ocean_atlas_monthly.nc;http://ferret.pmel.noaa.gov/thredds/dodsC/data/PMEL/ocean_atlas_monthly.nc \
coads_wspd.cdf;http://ferret.pmel.noaa.gov/thredds/dodsC/data/PMEL/COADS/coads_wspd.cdf \
duacs_global_nrt_msla_merged_h_lr;http://opendap.aviso.oceanobs.com/thredds/dodsC/duacs_global_nrt_msla_merged_h_lr \
"

UNUSED="\
NCAR_Abiotic_equil.nc;http://dods.ipsl.jussieu.fr/cgi-bin/nph-dods/ocmip/phase2/NCAR/Abiotic/equil/NCAR_Abiotic_equil.nc \
NCAR_Biotic_equil.nc;http://dods.ipsl.jussieu.fr/cgi-bin/nph-dods/ocmip/phase2/NCAR/Biotic/NCAR_Biotic_equil.nc \
"

XSPECIALTESTS="\
NCAR_Abiotic_equil.nc;http://dods.ipsl.jussieu.fr/cgi-bin/nph-dods/ocmip/phase2/NCAR/Abiotic/equil/NCAR_Abiotic_equil.nc \
NCAR_Biotic_equil.nc;http://dods.ipsl.jussieu.fr/cgi-bin/nph-dods/ocmip/phase2/NCAR/Biotic/NCAR_Biotic_equil.nc \
O2_ann_nobs_1deg.nc;http://ferret.pmel.noaa.gov/thredds/dodsC/data/PMEL/WOA01/english/annual/O2_ann_nobs_1deg.nc \
500m;http://ferret.pmel.noaa.gov/thredds/dodsC/hfrnet/agg/500m \
qscat_high_wind;http://apdrc.soest.hawaii.edu/dods/public_data/satellite_product/QSCAT/qscat_high_wind \
TOMS-EP_L3-TOMSEPL3_1997m0107_v8.HDF;http://disc1.sci.gsfc.nasa.gov/opendap/Earth_Probe_TOMS_Level3/TOMSEPL3/1997/TOMS-EP_L3-TOMSEPL3_1997m0107_v8.HDF \
ssta.dods;http://iridl.ldeo.columbia.edu/SOURCES/.ENSOFORECAST/.LDEO2/.12MON/.ssta/dods \
dods;http://ingrid.ldeo.columbia.edu/SOURCES/.OBERHUBER/dods \
d055.nc;http://ilikai.soest.hawaii.edu/cgi-bin/nph-dods/fast/d055.nc \
"

# Temporarily suppress
XFAILTESTS="qscat_high_wind totalAagg  world-unfilter-monthly.nc duacs_global_nrt_msla_merged_h_lr 500m"

RESULTSDIR="./results"
# Locate some tools
NCDUMP="${builddir}/ncdump/ncdump"
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

for ext in $EXTSET ; do

CONLY=
if test "x$ext" = xdmp ; then CONLY="-c"; fi

cd ${RESULTSDIR}
for t in ${SPECIALTESTS} ; do
  name=`echo $t | cut -d ';' -f1`
  url=`echo $t | cut -d ';' -f2`
  url="${PARAMS}${url}"
  if test "x$quiet" = "x0" ; then echo "*** Testing: ${name}"; fi
  if test "x$quiet" = "x0" ; then echo "*** URL: ${url}"; fi

  # Ignore missing cases
  if test -f ${EXPECTED}/${name}.${ext}
  then ignore=1; else echo "Missing: ${EXPECTED}/${name}.${ext}";  continue; fi

  # determine if this is an xfailtest
  isxfail=0
  for x in ${XFAILTESTS} ; do
    if test "x${name}" = "x${x}" ; then isxfail=1; fi
  done

  status=0

  if test "$ext" = hdr ; then H="-h" ; else H=""; fi

  if ${VALGRIND} ${NCDUMP} $CONLY $H ${url} > ${name}.${ext}
  then status=$status; else status=1; fi

  sort <${name}.${ext} >${name}.s${ext}
  sort <${EXPECTED}/${name}.${ext} >${name}.s${ext}.expected

  if diff -w ${name}.s${ext}.expected ${name}.s${ext} # compare with expected
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
done

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
