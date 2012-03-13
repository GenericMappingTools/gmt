#!/bin/bash
#
#       $Id$

. ./functions.sh
header "Testing rotconverter for adding rotations"

# We are testing a few examples from Cox and Hart, Plate Tectonics - How it works.

cat << EOF > answers.txt
157.32/-80.44/11.97
162.38/-72.57/11.62
EOF
rotconverter 150.1/70.5/-20.3 + 145/40/11.4 --FORMAT_GEO_OUT=D --FORMAT_FLOAT_OUT=%.2f --IO_COL_SEPARATOR=/ > results.txt
rotconverter 351.4/80.8/-22.5 + 62.26/85.36/11.14 --FORMAT_GEO_OUT=D --FORMAT_FLOAT_OUT=%.2f --IO_COL_SEPARATOR=/ >> results.txt

diff -q --strip-trailing-cr results.txt answers.txt >> fail
passfail spotter_6
