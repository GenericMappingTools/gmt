#!/usr/bin/env bash
#
# Bash script to install GMT dependencies on Ubuntu via apt-get and conda
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
# packages installed via conda
conda_packages="ghostscript=9.56.1"

# optional packages
if [ "$EXCLUDE_OPTIONAL" = "false" ]; then
    packages+=" libfftw3-dev libpcre3-dev liblapack-dev libglib2.0-dev"
fi

# packages for running GMT tests
if [ "$RUN_TESTS" = "true" ]; then
    packages+=" graphicsmagick gdal-bin texlive-latex-base texlive-binaries texlive-fonts-recommended"
    conda_packages+=" dvc"
fi

# packages for building documentation
if [ "$BUILD_DOCS" = "true" ]; then
	conda_packages+=" pngquant sphinx dvc"
fi

# Install packages via apt-get
sudo apt-get update
sudo apt-get install -y --no-install-recommends --no-install-suggests $packages

# Install packages via conda
conda update -n base -c conda-forge conda --solver libmamba
conda install ${conda_packages} -c conda-forge --solver libmamba
echo "${CONDA}/bin" >> $GITHUB_PATH

set +x +e
