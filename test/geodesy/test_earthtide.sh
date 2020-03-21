#!/usr/bin/env bash
#
# Tests gmt earthtide against the values of the original FORTRAN version


rm -f test_earthtide.dat

# Skip the first column with the time since the fortran version prints seconds since t0
gmt earthtide -T2018-7-7T/2018-7-7T00:04:00/1m -L0/0  --FORMAT_FLOAT_OUT=%.6f -o1:3 > test_earthtide.dat

# Output from the Fortran version
#-0.015781 -0.012347 -0.027025
#-0.015756 -0.012132 -0.027402
#-0.015731 -0.011916 -0.027772
#-0.015705 -0.011698 -0.028136
#-0.015678 -0.011480 -0.028494

diff test_earthtide.dat "${src:-.}"/test_earthtide.dat --strip-trailing-cr > fail
