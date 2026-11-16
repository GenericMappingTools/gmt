#!/usr/bin/env bash
#
# Longitudes far outside -180/360 must be wrapped without looping 360 degrees at a time:
# that took |lon|/360 steps and never ended once lon - 360 == lon (from about 5e18 on).
# The ellipsoidal Transverse Mercator inverse returns such longitudes for points far from
# the central meridian, where its series no longer converges, so mapproject -I did not return.

: > fail
# Input side (gmtio_adjust_periodic_lon): 1e20 = 280 (mod 360), -1e20 = 80 (mod 360)
echo "1e20 10" | gmt convert -fg --FORMAT_FLOAT_OUT=%.17g | $AWK '{if ($1 != -80 || $2 != 10) print "convert 1e20: got", $0}' >> fail
echo "-1e20 10" | gmt convert -fg --FORMAT_FLOAT_OUT=%.17g | $AWK '{if ($1 != 80 || $2 != 10) print "convert -1e20: got", $0}' >> fail
echo "1000 10" | gmt convert -fg --FORMAT_FLOAT_OUT=%.17g | $AWK '{if ($1 != -80 || $2 != 10) print "convert 1000: got", $0}' >> fail
# Output side (gmt_lon_range_adjust): a point far from the central meridian of the ellipsoidal TM inverse.
# Only that it returns one record is tested; the series gives no meaningful position there.
echo "-8493027.4968201965 10043801.564664664" | gmt mapproject -Jt0/1:1 -Rg -C -F -I 2> /dev/null | $AWK 'END {if (NR != 1) print "mapproject -Jt -I: got", NR, "records"}' >> fail
