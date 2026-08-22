#!/usr/bin/env bash
#
# Bash script to install GMT dependencies on macOS via Homebrew and micromamba
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
mamba_packages="ghostscript=10.03.0"

# packages for build documentation
if [ "$BUILD_DOCS" = "true" ]; then
    packages+=" pngquant"
    mamba_packages+=" sphinx dvc"
fi

# packages for running GMT tests
if [ "$RUN_TESTS" = "true" ]; then
    packages+=" graphicsmagick"
    mamba_packages+=" dvc"
fi

if [ "$PACKAGE" = "true" ]; then
    # we need the GNU tar for packaging
    packages+=" gnu-tar"
fi

# Install GMT dependencies
#brew update
brew install ${packages}

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
