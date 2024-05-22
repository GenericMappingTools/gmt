#!/usr/bin/env bash
#
# Run some simple GMT commands
#

set -x -e

# Check GMT splash screen
gmt

# Check GMT configuration
gmt-config --all

# Check GMT defaults
gmt defaults -Vd

# Check GMT classic mode, GSHHG and DCW
gmt pscoast -R0/10/0/10 -JM6i -Ba -Ggray -ENG+p1p,blue -P -Vd > test.ps

# Check GMT modern mode, GSHHG and DCW
if [ "${RUNNER_OS}" == "Windows" ]; then export GMT_SESSION_NAME=$$; fi
gmt begin && gmt coast -R0/10/0/10 -JM6i -Ba -Ggray -ENG+p1p,blue -Vd && gmt end
if [ "${RUNNER_OS}" == "Windows" ]; then unset GMT_SESSION_NAME; fi

# Check remote file and modern one-liner
gmt grdimage @earth_relief_01d -JH10c -Baf -pdf map

# Check supplemental modules
gmt earthtide -T2018-06-18T12:00:00 -Gsolid_tide_up.grd

# Check OpenMP support
gmt grdsample @earth_relief_01d -R0/20/0/20 -I30m -Gtopo_30m.nc -x2

set +x +e
