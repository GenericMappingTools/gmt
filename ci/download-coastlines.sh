#!/usr/bin/env bash
# Download and install the coastlines and boundaries datasets

# To return a failure if any commands inside fail
set -e

# Get the coastlines and country polygons
EXT="tar.gz"
GSHHG="gshhg-gmt-2.3.7"
DCW="dcw-gmt-1.1.4"

# GSHHG (coastlines, rivers, and political boundaries):
echo ""
echo "Downloading and unpacking GSHHG"
echo "================================================================================"
curl "http://www.soest.hawaii.edu/pwessel/gshhg/$GSHHG.$EXT" > $GSHHG.$EXT
tar xzf $GSHHG.$EXT
cp $GSHHG/* $COASTLINEDIR/

# DCW (country polygons):
echo ""
echo "Downloading and unpacking DCW"
echo "================================================================================"
curl "http://www.soest.hawaii.edu/pwessel/dcw/$DCW.$EXT" > $DCW.$EXT
tar xzf $DCW.$EXT
cp $DCW/* $COASTLINEDIR

ls $COASTLINEDIR

# Turn off exit on failure.
set +e
