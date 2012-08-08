# $Id$

##################################################
# Local test info
##################################################

# Define the complete URLS

FILEURL="file://${testdata3}"

SYNTHETICDATA="synth1 synth2 synth3 synth4 synth5 synth6 synth7"

ACTUALDATA1=\
"1990-S1700101.HDF.WVC_Lat 1998-6-avhrr.dat \
b31a b31 D1 Drifters \
EOSDB ingrid nestedDAS NestedSeq2 \
NestedSeq OverideExample pbug0001b SimpleDrdsExample \
test.01 test.02 test.03 test.04 \
test.05 test.06a test.07a \
test.07 test.21 test.22 \
test.23 test.31 \
test.50 test.53 test.55 \
test.56 test.57 \
test.66 test.67 test.68 test.69 \
test.an1 \
test.dfp1 test.dfr1 test.dfr2 test.dfr3 \
test.gr1 test.gr2 test.gr3 test.gr4 \
test.gr5 test.PointFile test.sds1 \
test.sds2 test.sds3 test.sds4 test.sds5 \
test.sds6 test.sds7 test.vs1 \
test.vs2 test.vs3 test.vs4 test.vs5 whoi"

ACTUALDATA2=\
"123bears.nc 123.nc bears.nc \
ber-2002-10-01.nc ceopL2AIRS2-2.nc \
data.nc fnoc1.nc \
in1.nc in_2.nc in.nc \
in_no_three_double_dmn.nc in_v.nc saco1.nc \
test.nc text.nc"

# XFAIL tests should be a subset of the other tests; this is used
# only to detect which are considered XFAIL tests.
XFAILTESTS=""

# For now, remove some tests from windows platform.
if [ `uname | cut -d "_" -f 1` = "MINGW32" ]; then
    XFAILTESTS="$XFAILTESTS EOSDB OverideExample SimpleDrdsExample test.67 test.gr5 123bears.nc 123.nc bears.nc ber-2002-10-01 data.nc in1.nc in_2.nc in_no_three_double_dmn.nc test.nc text.nc test.22 test.23 test.gr1 in.nc ber-2002-10-01.nc"
fi

FILETESTS="${SYNTHETICDATA} ${ACTUALDATA1} ${ACTUALDATA2}"

