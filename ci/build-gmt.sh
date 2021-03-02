#!/usr/bin/env bash
#
# Build the GMT source codes of a specific branch or tag/release.
#
# Usage:
#
#    bash build-gmt.sh [branch-or-tag]
#
# Environmental variables that controls the building process:
#
# - GMT_INSTALL_DIR : GMT installation location [$HOME/gmt-install-dir]
# - GMT_GIT_REF     : branch name or tag/release [master if CLI argument isn't given]
#
# Notes for CI service users:
#
# 1. Install GMT dependencies following the [wiki](https://github.com/GenericMappingTools/gmt/wiki)
# 2. Run the following command to build the GMT source codes:
#
#      curl https://raw.githubusercontent.com/GenericMappingTools/gmt/master/ci/build-gmt.sh | bash
#

set -x -e

# Following variables can be modified via environment variables
GMT_INSTALL_DIR=${GMT_INSTALL_DIR:-${HOME}/gmt-install-dir}
if [ "X$1" = "X" ]; then
	GMT_GIT_REF=${GMT_GIT_REF:-master}
else
	GMT_GIT_REF=$1
fi

# General settings
GSHHG_VERSION="2.3.7"
DCW_VERSION="1.1.4"
GSHHG="gshhg-gmt-${GSHHG_VERSION}"
DCW="dcw-gmt-${DCW_VERSION}"
EXT="tar.gz"

cwd=${PWD}
# 1. Create temporary directory for building
GMT_BUILD_TMPDIR=$(mktemp -d ${TMPDIR:-/tmp/}gmt.XXXXXX)
cd ${GMT_BUILD_TMPDIR}

# 2. Download GMT, GSHHG and DCW from GitHub
git clone --depth=1 --single-branch --branch ${GMT_GIT_REF} https://github.com/GenericMappingTools/gmt
curl -SLO https://github.com/GenericMappingTools/gshhg-gmt/releases/download/${GSHHG_VERSION}/${GSHHG}.${EXT}
curl -SLO https://github.com/GenericMappingTools/dcw-gmt/releases/download/${DCW_VERSION}/${DCW}.${EXT}

# 3. Extract tarballs
tar -xvf ${GSHHG}.${EXT}
tar -xvf ${DCW}.${EXT}
mv ${GSHHG} gmt/share/gshhg-gmt
mv ${DCW} gmt/share/dcw-gmt

# 4. Configure GMT
cd gmt/
cat > cmake/ConfigUser.cmake << EOF
set (CMAKE_INSTALL_PREFIX "${GMT_INSTALL_DIR}")
set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement \${CMAKE_C_FLAGS}")
set (CMAKE_C_FLAGS "-Wextra \${CMAKE_C_FLAGS}")
EOF

# 5. Build and install GMT
mkdir build
cd build
if command -v ninja >/dev/null 2>&1 ; then
	cmake .. -G Ninja
else
	cmake ..
fi
cmake --build .
cmake --build . --target install

# 6. Cleanup
cd ${cwd}
rm -rf ${GMT_BUILD_TMPDIR}

set -x -e
