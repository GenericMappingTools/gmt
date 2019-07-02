#!/bin/bash
# Script that builds a GMT release and makes the compressed tarballs.
# If run under macOS it also builds the macOS Bundle
if [ $# -gt 0 ]; then
	echo "Usage: build-release.sh"
	echo "build-release.sh must be run from top-level gmt directory"
	echo "Will create the release compressed tarballs and (under macOS) the bundle"
	echo "Requires you have set GMT_PACKAGE_VERSION_* and GMT_PUBLIC_RELEASE in cmake/ConfigDefaults.cmake"
	exit 1
fi
if [ ! -d cmake ]; then
	echo "Must be run from top-level gmt directory"
	exit 1
fi

# 1. Set basic ConfigUser.cmake file for a release build
if [ -f cmake/ConfigUser.cmake ]; then
	cp cmake/ConfigUser.cmake cmake/ConfigUser.cmake.orig
fi
cp -f admin/ConfigReleaseBuild.cmake cmake/ConfigUser.cmake
# 2. Make build dir and configure it
rm -rf build
mkdir build
cd build
echo "build-release.sh: Configure and build tar balls"
cmake -G Ninja ..
# 3. Build the release and the tar balls
cmake --build . --target gmt_release
cmake --build . --target gmt_release_tar
# 4. get the version string
Version=`src/gmt --version`
# 5. Remove the uncompressed tar ball
rm -f gmt-${Version}-src.tar
# 6. Install executables before building macOS Bundle
cmake --build . --target install
if [ `uname` = "Darwin" ]; then
	echo "build-release.sh: Build macOS bundle"
	# 7. Build the macOS Bundle
	cpack -G Bundle
fi
# 8. Report m5d hash
echo "build-release.sh: Report m5d hash per file"
md5 gmt-${Version}-*
# 9. Replace temporary ConfigReleaseBuild.cmake file with the original file
rm -f ../cmake/ConfigUser.cmake
if [ -f ../cmake/ConfigUser.cmake.orig ]; then
	mv ../cmake/ConfigUser.cmake.orig ../cmake/ConfigUser.cmake
fi
# 10. Put the products on my ftp site
#scp gmt-${Version}-darwin-x86_64.dmg gmt-${Version}-src.tar.* ftp:/export/ftp1/ftp/pub/pwessel/release
