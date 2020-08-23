#!/usr/bin/env bash
#
# Bash script to install GMT dependencies on macOS via Homebrew
#
# Environmental variables that can control the installation:
#
# - BUILD_DOCS: Build GMT documentations [false]
# - RUN_TESTS:  Run GMT tests            [false]
#
set -x -e

# set defaults to false
BUILD_DOCS="${BUILD_DOCS:-false}"
RUN_TESTS="${RUN_TESTS:-false}"

# packages for compiling GMT
# cmake is pre-installed on GitHub Actions
packages="ninja curl pcre2 netcdf gdal fftw ghostscript"

# packages for build documentations
if [ "$BUILD_DOCS" = "true" ]; then
    packages+=" graphicsmagick ffmpeg pngquant"
fi
# packages for running GMT tests
if [ "$RUN_TESTS" = "true" ]; then
    packages+=" graphicsmagick"
fi

if [ "$PACKAGE" = "true" ]; then
    # we need the GNU tar for packaging
	packages+=" gnu-tar"
fi

# Install packages
brew update
brew install ${packages}

if [ "$BUILD_DOCS" = "true" ]; then
	pip3 install --user sphinx
fi
