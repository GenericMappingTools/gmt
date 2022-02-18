#!/usr/bin/env bash
#
# Build a cache file for building GMT with OpenMP

CLANG_V=$1
DISTRO=$2

if [ ${DISTRO} = "MacPorts" ]; then
	top=/opt/local
    clang="/opt/local/bin/clang-mp-${CLANG_V}"
    clangxx="/opt/local/bin/clang++-mp-${CLANG_V}"
elif [ ${DISTRO} = "HomeBrew" ]; then
	top=/usr/local
    clang="/usr/local/opt/llvm@${CLANG_V}/bin/clang-${CLANG_V}"
    clangxx="/usr/local/opt/llvm@${CLANG_V}/bin/clang-${CLANG_V}"
else	# Requires either MacPorts of HomeBrew
	exit 1
fi

COMPC=$(which ${clang})
if ! [ "X${COMPC}" = "X" ]; then	
	# clang-mp-${CLANG_V} is needed to build with OpenMP
	if ! [ -x "$(command -v ${clang})" ]; then
		echo "build-release.sh: Error: clang-mp-${CLANG_V} is not found in your search PATH." >&2
		exit 1
	fi
	# clang++-mp-${CLANG_V} is needed to build with OpenMP
	if ! [ -x "$(command -v ${clangxx})" ]; then
		echo "build-release.sh: Error: clang++-mp-${CLANG_V} is not found in your search PATH." >&2
		exit 1
	fi

	cat <<- EOF > cache-mp-clang.cmake
	# Cache settings for building the macOS release with Clang and OpenMP
	# This cache file is set for the binary paths of macports or homebrew
	#
	SET ( CMAKE_C_COMPILER ${clang} CACHE STRING "CLANG MP C compiler" )
	SET ( CMAKE_CXX_COMPILER ${clangxx} CACHE STRING "CLANG MP C++ compiler" )
	SET ( CMAKE_C_FLAGS -flax-vector-conversions CACHE STRING "C FLAGS")
	SET ( CMAKE_C_FLAGS_DEBUG -flax-vector-conversions CACHE STRING "C FLAGS DEBUG")
	SET ( CMAKE_C_FLAGS_RELEASE -flax-vector-conversions CACHE STRING "C FLAGS RELEASE")
	SET ( OpenMP_C_FLAGS -flax-vector-conversions CACHE STRING "C FLAGS OPENMP")
	EOF
fi
