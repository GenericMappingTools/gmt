#!/usr/bin/env bash
#
# Compile and link the gmtmex shared library for the user's (most recent) MATLAB version
MATLAB_VERSION=$(ls /Applications | grep MATLAB | sort -r | head -1)
if [ "X${MATLAB_VERSION}" = "X" ]; then
	printf "MATLAB not found in /Applications - exiting.\n" >&2
	exit -1
fi

printf "\ngmt_mexbuild.sh found most recent MATLAB application: %s\n" ${MATLAB_VERSION} >&2
printf "\ngmt_mexbuild.sh will build and place the GMT/MEX toolbox in %s.\n" ${BUNDLE_RESOURCES}/bin >&2
printf "You must have sudo privileges on this computer.\n\nContinue? (y/n) [y]:" >&2
read answer
if [ "X$answer" = "Xn" ]; then
	exit 0
fi

# Make all the shared libraries use rpath instead of executable path
find . -name '*.dylib' > /tmp/raw.lis
rm -f /tmp/lib.lis
while read file; do
	if [ ! -L $file ]; then
		echo $file >> /tmp/lib.lis
	fi
done < /tmp/raw.lis
# For each library, replace @executable_path/../lib/*.dylib with @rpath/lib*.dylib
while read file; do
	echo $file
	otool -L $file | grep '@executable_path' | tr '/' ' ' | awk '{printf "install_name_tool -change @executable_path/../lib/%s @rpath/%s %s\n", $4, $4, "'$file'"}' | sh -s
done < /tmp/lib.lis

# Build the gmtmex executable
type=$(uname -m)
xcrun clang -I${BUNDLE_RESOURCES}/include/gmt -I/Applications/${MATLAB_VERSION}/extern/include -m64 -fPIC -fno-strict-aliasing -std=c99 -DGMT_MATLAB -c ${BUNDLE_RESOURCES}/share/tools/gmtmex.c -o /tmp/gmtmex.o
xcrun clang -undefined error -arch ${type} -bundle /tmp/gmtmex.o -L${BUNDLE_RESOURCES}/lib -lgmt -L/Applications/${MATLAB_VERSION}/bin/maci64 -lmx -lmex -rpath ${BUNDLE_RESOURCES}/lib -o ${BUNDLE_RESOURCES}/bin/gmtmex.mexmaci64
cp -f ${BUNDLE_RESOURCES}/share/tools/gmt.m ${BUNDLE_RESOURCES}/bin
printf "You must add this path to your MATLAB path: %s\n" ${BUNDLE_RESOURCES}/bin >&2
