#!/usr/bin/env bash
#
# Bash script to install GMT dependencies on Ubuntu via apt-get and micromamba
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

# packages installed via apt-get
packages="build-essential cmake ninja-build libcurl4-gnutls-dev libnetcdf-dev curl git libgdal-dev"
# packages installed via micromamba
mamba_packages="ghostscript=10.03.0"

# optional packages
if [ "$EXCLUDE_OPTIONAL" = "false" ]; then
    packages+=" libfftw3-dev libpcre2-dev liblapack-dev libglib2.0-dev"
fi

# packages for running GMT tests
if [ "$RUN_TESTS" = "true" ]; then
    packages+=" graphicsmagick gdal-bin texlive-latex-base texlive-binaries texlive-fonts-recommended"
fi

# packages for building documentation
if [ "$BUILD_DOCS" = "true" ]; then
    mamba_packages+=" pngquant sphinx pip"
fi

# Install packages via apt-get
sudo apt-get update
sudo apt-get install -y --no-install-recommends --no-install-suggests $packages

# Install packages via micromamba into a dedicated environment
micromamba create --yes --name gmt ${mamba_packages} -c conda-forge
MAMBA_PREFIX="${MAMBA_ROOT_PREFIX}/envs/gmt"
echo "${MAMBA_PREFIX}/bin" >> $GITHUB_PATH

# Remove pcre-config from micromamba's path so cmake won't find the micromamba's one
rm -f ${MAMBA_PREFIX}/bin/pcre-config ${MAMBA_PREFIX}/bin/pcre2-config

# Install Sphinx extensions
if [ "$BUILD_DOCS" = "true" ]; then
    ${MAMBA_PREFIX}/bin/python -m pip install --user -r doc/rst/requirements.txt
fi

set +x +e
