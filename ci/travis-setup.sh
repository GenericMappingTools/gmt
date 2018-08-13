#!/bin/bash
# Setup TravisCI to be able to build and test GMT

# To return a failure if any commands inside fail
set -e

# Install dependencies
if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    sudo apt-get update
    sudo apt-get install -y build-essential cmake libcurl4-gnutls-dev libnetcdf-dev \
        libgdal1-dev libfftw3-dev libpcre3-dev liblapack-dev ghostscript curl
else
    echo "OSX not supported yet";
fi

# Get the coastlines and country polygons
EXT="tar.gz"
GSHHG="gshhg-gmt-2.3.7"
DCW="dcw-gmt-1.1.4"

mkdir $INSTALLDIR
mkdir $COASTLINEDIR

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

# Turn off exit on failure.
set +e
