#!/usr/bin/env bash
#
# Bash script to install GMT dependencies on macOS via Homebrew and conda
#
# Environmental variables that can control the installation:
#
# - BUILD_DOCS: Build GMT documentation  [false]
# - RUN_TESTS:  Run GMT tests            [false]
# - PACKAGE:    Create GMT packages      [false]
#
set -x -e

# set defaults to false
BUILD_DOCS="${BUILD_DOCS:-false}"
RUN_TESTS="${RUN_TESTS:-false}"
PACKAGE="${PACKAGE:-false}"

# packages for compiling GMT
# cmake is pre-installed on GitHub Actions
packages="ninja curl pcre2 netcdf gdal geos fftw libomp"
conda_packages="ghostscript=10.02.0"

# packages for build documentation
if [ "$BUILD_DOCS" = "true" ]; then
    packages+=" dvc pngquant"
    conda_packages+=" sphinx"
fi

# packages for running GMT tests
if [ "$RUN_TESTS" = "true" ]; then
    packages+=" dvc graphicsmagick"
fi

if [ "$PACKAGE" = "true" ]; then
    # we need the GNU tar for packaging
    packages+=" gnu-tar"
fi

# Install GMT dependencies
#brew update
brew install ${packages}

# Install packages via conda
conda update -n base -c conda-forge conda --solver libmamba
conda install ${conda_packages} -c conda-forge --solver libmamba
echo "${CONDA}/bin" >> $GITHUB_PATH

set +x +e
