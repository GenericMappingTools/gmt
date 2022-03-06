#!/usr/bin/env bash
#
# Bash script to install GMT dependencies on Windows via vcpkg and chocolatey
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

WIN_PLATFORM=x64-windows

# install libraries
vcpkg install netcdf-c gdal[core,tools] pcre2 fftw3[core,threads] clapack openblas --triplet ${WIN_PLATFORM}
# Executable files search for DLL files in the directories listed in the PATH environment variable.
echo "${VCPKG_INSTALLATION_ROOT}/installed/${WIN_PLATFORM}/bin" >> $GITHUB_PATH
# Tools like gdal_translate, ogr2ogr are located in tools/gdal
echo "${VCPKG_INSTALLATION_ROOT}/installed/${WIN_PLATFORM}/tools/gdal" >> $GITHUB_PATH

# list installed packages
vcpkg list

# install more packages using chocolatey
choco install ninja
choco install ghostscript --version 9.50

if [ "$BUILD_DOCS" = "true" ]; then
    pip install --user sphinx dvc
    # Add sphinx to PATH
    echo "$(python -m site --user-site)\..\Scripts" >> $GITHUB_PATH

    choco install pngquant
fi

if [ "$RUN_TESTS" = "true" ]; then
    choco install graphicsmagick --version 1.3.32
    pip install --user dvc
    # Add GraphicsMagick to PATH
    echo 'C:\Program Files\GraphicsMagick-1.3.32-Q8' >> $GITHUB_PATH
    # Add dvc to PATH
    echo "$(python -m site --user-site)\..\Scripts" >> $GITHUB_PATH
fi

# we need the GNU tar for packaging
if [ "$PACKAGE" = "true" ]; then
    echo 'C:\Program Files\Git\usr\bin\' >> $GITHUB_PATH
fi

set +x +e
