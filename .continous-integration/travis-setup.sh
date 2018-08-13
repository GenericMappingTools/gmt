#!/bin/bash
# Setup TravisCI to be able to build and test GMT

# Install dependencies
sudo apt-get install build-essential cmake libcurl4-gnutls-dev libnetcdf-dev \
    libgdal1-dev libfftw3-dev libpcre3-dev liblapack-dev ghostscript

# Get the coastlines and country polygons
EXT="tar.gz"
GSHHG="gshhg-gmt-2.3.6"
DCW="dcw-gmt-1.1.2"

echo ""
echo "Creating folder $COASTLINEDIR for the coastline and coutry data."
echo ""
mkdir $COASTLINEDIR

# GSHHG (coastlines, rivers, and political boundaries):
echo ""
echo "Downloading and unpacking GSHHG"
echo "================================================================================"
curl "ftp://ftp.iag.usp.br/pub/gmt/$GSHHG.$EXT" > $GSHHG.$EXT
tar xzf $GSHHG.$EXT
cp $GSHHG/* $COASTLINEDIR/

# DCW (country polygons):
echo ""
echo "Downloading and unpacking DCW"
echo "================================================================================"
curl "ftp://ftp.soest.hawaii.edu/dcw/$DCW.$EXT" > $DCW.$EXT
tar xzf $DCW.$EXT
cp $DCW/* $COASTLINEDIR
