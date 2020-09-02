#!/usr/bin/env bash
#
# Script that builds a GMT release and makes the compressed tarballs.
# If run under macOS it also builds the macOS Bundle.
#
# Requirements in addition to libraries and tools to build GMT:
#	1) pngquant for squeezing PNG files down in size
#	2) GMT_GSHHG_SOURCE and GMT_DCW_SOURCE environmental parameters set
#	3) A ghostscript version we can include in the macOS bundle [MacPort]
#	4) sphinx-build 
#	5) grealpath (package coreutils)
#	6) GNU tar (package gnutar)

reset_config() {
	rm -f ${TOPDIR}/cmake/ConfigUser.cmake
	if [ -f ${TOPDIR}/cmake/ConfigUser.cmake.orig ]; then # Restore what we had
		mv -f ${TOPDIR}/cmake/ConfigUser.cmake.orig ${TOPDIR}/cmake/ConfigUser.cmake
	fi
}

abort_build() {	# Called when we abort this script via Crtl-C
	echo "build-release.sh: Detected Ctrl-C - aborting" >&2
	reset_config
	rm -rf ${TOPDIR}/build
	exit -1
}

TOPDIR=$(pwd)

if [ $# -gt 0 ]; then
	cat <<- EOF  >&2
	Usage: build-release.sh
	
	build-release.sh must be run from top-level gmt directory.
	Will create the release compressed tarballs and (under macOS) the bundle.
	Requires you have set GMT_PACKAGE_VERSION_* and GMT_PUBLIC_RELEASE in cmake/ConfigDefaults.cmake.
	Requires GMT_GSHHG_SOURCE and GMT_DCW_SOURCE to be set in the environment.
	EOF
	exit 1
fi
if [ ! -d cmake ]; then
	echo "build-release.sh: Must be run from top-level gmt directory" >&2
	exit 1
fi

# pngquant is need to optimize images
if ! [ -x "$(command -v gmt)" ]; then
	echo 'build-release.sh: Error: pngquant is not found in your search PATH.' >&2
	exit 1
fi

# 0. Make sure GMT_GSHHG_SOURCE and GMT_DCW_SOURCE are set in the environment
if [ "X${GMT_GSHHG_SOURCE}" = "X" ]; then
	echo "build-release.sh: Need to setenv GMT_GSHHG_SOURCE to point to directory with GSHHG files" >&2
	exit 1
fi
if [ "X${GMT_DCW_SOURCE}" = "X" ]; then
	echo "build-release.sh: Need to setenv GMT_DCW_SOURCE to point to directory with DCW files" >&2
	exit 1
fi

if [ $(egrep -c '^set \(GMT_PUBLIC_RELEASE TRUE\)' cmake/ConfigDefault.cmake) -eq 0 ]; then
	echo "build-release.sh: Need to set GMT_PUBLIC_RELEASE to TRUE in cmake/ConfigDefault.cmake" >&2
	exit 1
fi

G_ver=$(gs --version)
echo "build-release.sh: You will be including Ghostscript version $G_ver"
echo "build-release.sh: Running admin/gs-check.sh to ensure it passes our transparency test" >&2
err=$(admin/gs_check.sh | grep Total | awk '{print $3}')
if [ "X${err}" = "X0.0" ]; then
	echo "build-release.sh: Ghostscript version $G_ver passed the test - will be included" >&2
else
	echo "build-release.sh: Ghostscript version $G_ver failed the test - install another gs" >&2
	exit 1
fi
# 1. Set basic ConfigUser.cmake file for a release build
if [ -f cmake/ConfigUser.cmake ]; then
	cp cmake/ConfigUser.cmake cmake/ConfigUser.cmake.orig
fi
cp -f admin/ConfigReleaseBuild.cmake cmake/ConfigUser.cmake
trap abort_build SIGINT
# 2a. Make build dir and configure it
rm -rf build
mkdir build
# 2b. Build list of external programs and shared libraries
admin/build-macos-external-list.sh > build/add_macOS_cpack.txt
cd build
# 2c. Set CMake cache for MP build:
cat << EOF > cache-mp-gcc.cmake
# Cache settings for building the macOS release with GCC and OpenMP
# This cache file is set for the binary paths of macports
#
SET ( CMAKE_C_COMPILER "/opt/local/bin/gcc-mp-9" CACHE STRING "GNU MP C compiler" )
SET ( CMAKE_CXX_COMPILER "/opt/local/bin/g++-mp-9" CACHE STRING "GNU MP C++ compiler" )
SET ( CMAKE_C_FLAGS -flax-vector-conversions CACHE STRING "C FLAGS")
SET ( CMAKE_C_FLAGS_DEBUG -flax-vector-conversions CACHE STRING "C FLAGS DEBUG")
SET ( CMAKE_C_FLAGS_RELEASE -flax-vector-conversions CACHE STRING "C FLAGS RELEASE")
SET ( OpenMP_C_FLAGS -flax-vector-conversions CACHE STRING "C FLAGS OPENMP")
EOF

echo "build-release.sh: Configure and build tarballs" >&2
cmake -G Ninja  -C cache-mp-gcc.cmake ..
# 3. Build the release and the tarballs
cmake --build . --target gmt_release
cmake --build . --target gmt_release_tar
# 4. get the version string
Version=$(src/gmt --version)
# 5. Remove the uncompressed tarball
rm -f gmt-${Version}-src.tar
# 6. Install executables before building macOS Bundle
cmake --build . --target install
if [ $(uname) = "Darwin" ]; then
	echo "build-release.sh: Build macOS bundle" >&2
	# 7. Build the macOS Bundle
	cpack -G Bundle
fi
# 8. Report sha256 hash
echo "build-release.sh: Report sha256sum per file" >&2
shasum -a 256 gmt-${Version}-*
# 9. Replace temporary ConfigReleaseBuild.cmake file with the original file
reset_config
# 10. Put the candidate products on my ftp site
echo "build-release.sh: Place gmt-${Version}-src.tar.* on the ftp site" >&2
if [ -f gmt-${Version}-darwin-x86_64.dmg ]; then
	echo "build-release.sh: Place gmt-${Version}-darwin-x86_64.dmg on the ftp site" >&2
fi
if [ "${USER}" = "pwessel" ]; then	# Place file in pwessel SOEST ftp release directory and set permissions
	scp gmt-${Version}-darwin-x86_64.dmg gmt-${Version}-src.tar.* ftp.soest.hawaii.edu:/export/ftp1/ftp/pub/pwessel/release
	ssh pwessel@ftp.soest.hawaii.edu 'chmod og+r /export/ftp1/ftp/pub/pwessel/release/gmt-*'
fi
