#!/usr/bin/env bash
#
# Build the lastest GMT source codes.
#
# Usage:
#
# 1. Install GMT dependencies following the [wiki](https://github.com/GenericMappingTools/gmt/wiki)
# 2. curl https://raw.githubusercontent.com/GenericMappingTools/gmt/master/ci/build-gmt-master.sh | bash
#
# Environmental variables that affect the building process:
#
# - GMT_INSTALL_DIR: GMT installation location
#

set -x -e

# Following variables can be modified via environment variables
GMT_INSTALL_DIR=${GMT_INSTALL_DIR:-${HOME}/gmt-install-dir}

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
git clone --depth 1 https://github.com/GenericMappingTools/gmt.git gmt
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
cmake ..
make
make install

# 6. Cleanup
cd ${cwd}
rm -rf ${GMT_BUILD_TMPDIR}

set -x -e
