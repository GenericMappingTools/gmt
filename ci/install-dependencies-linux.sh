#!/usr/bin/env bash
#
# Bash script to install GMT dependencies on Ubuntu.
#
# Environmental variables that can control the installation:
#
# - BUILD_DOCS: Build GMT documentation                 [false]
# - RUN_TESTS: Run GMT tests                            [false]
# - EXCLUDE_OPTIONAL: Exclude optional dependencies     [false]
#
set -x -e

# set defaults to false
BUILD_DOCS="${BUILD_DOCS:-false}"
RUN_TESTS="${RUN_TESTS:-false}"
EXCLUDE_OPTIONAL=${EXCLUDE_OPTIONAL:-false}

# required packages for compiling GMT
packages="build-essential cmake ninja-build libcurl4-gnutls-dev libnetcdf-dev \
          ghostscript curl git"

# optional packages
if [ "$EXCLUDE_OPTIONAL" = "false" ]; then
    packages+=" libgdal-dev libfftw3-dev libpcre3-dev liblapack-dev libglib2.0-dev"
fi

# packages for building documentation
if [ "$BUILD_DOCS" = "true" ]; then
    packages+=" python3-pip python3-setuptools python3-wheel graphicsmagick ffmpeg"
fi

# packages for running GMT tests
if [ "$RUN_TESTS" = "true" ]; then
    packages+=" graphicsmagick gdal-bin"
fi

# Install packages
sudo apt-get update
sudo apt-get install -y --no-install-recommends --no-install-suggests $packages

# Install more packages for building documentation
if [ "$BUILD_DOCS" = "true" ]; then
    sudo snap install pngquant
    pip3 install --user sphinx
    # Add sphinx to PATH
    echo "$(python3 -m site --user-base)/bin" >> $GITHUB_PATH
fi

set +x +e
