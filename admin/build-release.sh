#!/bin/bash
# Script that builds a GMT release and makes the compressed tarballs and macOS Bundle
Version=6.0.0
if [ $# -eq 0 ]; then
	echo "Usage: build-release.sh tag"
	echo "e.g., build-release.sh rc3"
	echo "If no tag (i.e., a final 6.x.y version) then give - as tag"
	echo "build-release.sh must be run from top-level gmt directory"
	exit 1
fi
if [ ! -d cmake ]; then
	echo "Must be run from top-level gmt directory"
	exit 1
fi
if [ ! "X$1" = "X-" ]; then
	tag=$1
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
cmake -G Ninja -DGMT_PACKAGE_VERSION_SUFFIX=${tag} -DGMT_PUBLIC_RELEASE=TRUE ..
# 3. Build the release and the tar balls
cmake --build . --target gmt_release
cmake --build . --target gmt_release_tar
# 4. Remove the uncompressed tar ball
rm -f gmt-${Version}${tag}-src.tar
# 5. Install executables before building macOS Bundle
cmake --build . --target install
# 6. Build the macOS Bundle
cpack -G Bundle
# 7. Report m5d hash
md5 gmt-${Version}${tag}-*
# 8. Replace temporary ConfigReleaseBuild.cmake file with the original file
rm -f ../cmake/ConfigUser.cmake
if [ -f ../cmake/ConfigUser.cmake.orig ]; then
	mv ../cmake/ConfigUser.cmake.orig cmake/ConfigUser.cmake
fi
# 10. Put the products on my ftp site
#scp gmt-${Version}${tag}-darwin-x86_64.dmg gmt-${Version}${tag}-src.tar.* ftp:/export/ftp1/ftp/pub/pwessel/release
