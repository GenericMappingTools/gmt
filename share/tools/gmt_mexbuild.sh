#!/usr/bin/env bash
#
# Compile and link the gmtmex shared library for the user's (most recent) MATLAB version

# 0. Check if already installed
if [ -f ${BUNDLE_RESOURCES}/bin/gmtmex.mexmaci64 ]; then
	printf "GMT/MEX toolbox already installed on this computer.\n" >&2
	printf "To reinstall you must first remove ${BUNDLE_RESOURCES}/bin/gmtmex.mexmaci64.\n" >&2
	exit 1
fi

# 1. Determine a single, most recent Matlab version
MATLAB_VERSION=$(ls /Applications | grep MATLAB | sort -r | head -1)
if [ "X${MATLAB_VERSION}" = "X" ]; then
	printf "MATLAB not found in /Applications - exiting.\n" >&2
	exit -1
fi

# 2. Determine that Xcode has been installed
if ! [ -x "$(command -v xcrun)" ]; then
	echo 'build-release.sh: Error: Xcode CLI tools (xcrun) not found in your search PATH.' >&2
	exit 1
fi

printf "\ngmt_mexbuild.sh found most recent MATLAB application: %s\n" ${MATLAB_VERSION} >&2
printf "\ngmt_mexbuild.sh will build and place the GMT/MEX toolbox in %s.\n" ${BUNDLE_RESOURCES}/bin >&2
# 3. If use does not have write permission, explain they must use sudo
if [ ! -w ${BUNDLE_RESOURCES} ]; then
	printf "You must have sudo privileges on this computer.\n\nContinue? (y/n) [y]:" >&2
	read answer
	if [ "X$answer" = "Xn" ]; then
		exit 0
	fi
fi

printf "\ngmt_mexbuild.sh: Working..." >&2
# 4. Make a listing of all shared libraries but skip symbolic links
find ${BUNDLE_RESOURCES}/lib -name '*.dylib' > /tmp/raw.lis
rm -f /tmp/lib.lis
while read file; do
	if [ ! -L $file ]; then	# Skipping symbolic links
		echo $file >> /tmp/lib.lis
	fi
done < /tmp/raw.lis
# 5. Make all shared libraries use rpath instead of executable path
#    For each library, replace @executable_path/../lib/*.dylib with @rpath/lib*.dylib
rm -f /tmp/mexjob.sh
while read file; do
	otool -L $file | grep '@executable_path' | tr '/' ' ' | awk '{printf "install_name_tool -change @executable_path/../lib/%s @rpath/%s %s\n", $4, $4, "'$file'"}' >> /tmp/mexjob.sh
done < /tmp/lib.lis
sh /tmp/mexjob.sh

# 6. Update the gmt executable to use rpath also
install_name_tool -add_rpath ${BUNDLE_RESOURCES}/lib gmt

# 7. Build the gmtmex executable
type=$(uname -m)
version=$(gmt --version | awk -Fr '{print $1}')	# Skip trailing stuff like rc1
xcrun clang -I${BUNDLE_RESOURCES}/include/gmt -I/Applications/${MATLAB_VERSION}/extern/include -m64 -fPIC -fno-strict-aliasing -std=c99 -DGMT_MATLAB -c ${BUNDLE_RESOURCES}/share/tools/gmtmex.c -o /tmp/gmtmex.o
xcrun clang -undefined error -arch ${type} -bundle /tmp/gmtmex.o -L${BUNDLE_RESOURCES}/lib -lgmt -L/Applications/${MATLAB_VERSION}/bin/maci64 -lmx -lmex -rpath ${BUNDLE_RESOURCES}/lib -o ${BUNDLE_RESOURCES}/bin/gmtmex.mexmaci64
cp -f ${BUNDLE_RESOURCES}/share/tools/gmt.m ${BUNDLE_RESOURCES}/bin
install_name_tool -change @executable_path/../lib/libgmt.${version}.dylib @rpath/libgmt.${version}.dylib ${BUNDLE_RESOURCES}/bin/gmtmex.mexmaci64
printf "done\n" >&2
printf "You must add this path to your MATLAB path: %s\n" ${BUNDLE_RESOURCES}/bin >&2
