#!/usr/bin/env bash
#
# Build the GMT source codes of a specific branch, version or commit
#
# Usage:
#
# 1. Install GMT dependencies following the [wiki](https://github.com/GenericMappingTools/gmt/wiki)
# 2. Run the following command to build the GMT source codes (master branch by default):
#
#      curl https://raw.githubusercontent.com/GenericMappingTools/gmt/master/ci/build-gmt.sh | bash
#
# Environmental variables that affect the building process:
#
# - GMT_INSTALL_DIR : GMT installation location [$HOME/gmt-install-dir]
# - GMT_GIT_REF     : branch name, version, or commit full sha [master]
#

set -x -e

# Following variables can be modified via environment variables
GMT_INSTALL_DIR=${GMT_INSTALL_DIR:-${HOME}/gmt-install-dir}
GMT_GIT_REF=${GMT_GIT_REF:-master}

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
curl -SLO https://github.com/GenericMappingTools/gmt/archive/${GMT_GIT_REF}.${EXT}
curl -SLO https://github.com/GenericMappingTools/gshhg-gmt/releases/download/${GSHHG_VERSION}/${GSHHG}.${EXT}
curl -SLO https://github.com/GenericMappingTools/dcw-gmt/releases/download/${DCW_VERSION}/${DCW}.${EXT}

# 3. Extract tarballs
tar -xvf ${GMT_GIT_REF}.${EXT}
tar -xvf ${GSHHG}.${EXT}
tar -xvf ${DCW}.${EXT}
mv ${GSHHG} gmt-${GMT_GIT_REF}/share/gshhg-gmt
mv ${DCW} gmt-${GMT_GIT_REF}/share/dcw-gmt

# 4. Configure GMT
cd gmt-${GMT_GIT_REF}/
cat > cmake/ConfigUser.cmake << EOF
set (CMAKE_INSTALL_PREFIX "${GMT_INSTALL_DIR}")
set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement \${CMAKE_C_FLAGS}")
set (CMAKE_C_FLAGS "-Wextra \${CMAKE_C_FLAGS}")
EOF

# 5. Build and install GMT
mkdir build
cd build
cmake ..
cmake --build .
cmake --build . --target install

# 6. Cleanup
cd ${cwd}
rm -rf ${GMT_BUILD_TMPDIR}

set -x -e
