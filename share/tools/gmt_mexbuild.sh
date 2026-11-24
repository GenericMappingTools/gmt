#!/usr/bin/env bash
#
# Compile and link the GMT/MEX toolbox with the user's (most recent) MATLAB version.
# This step must be done by the user since we cannot distribute MATLAB, and the user
# needs to have a MATLAB license.  The requirements to build the GMT/MEX toolbox are:
#
#  1. A recent MATLAB installation in /Applications
#  2. A recent installation of Xcode command line tools (CLI)
#------------------------------------------------------------------------------------

# 0. Check if already installed
if [ -f ${BUNDLE_RESOURCES}/bin/gmtmex.mexmaci64 ]; then
	printf "gmt_mexbuild.sh: GMT/MEX toolbox already installed on this computer.\n" >&2
	printf "gmt_mexbuild.sh: To reinstall you must first remove this file:\n${BUNDLE_RESOURCES}/bin/gmtmex.mexmaci64.\n" >&2
	exit 1
fi

# 1. Determine a single, most recent Matlab version
MATLAB_VERSION=$(ls /Applications | grep MATLAB | sort -r | head -1)
if [ "X${MATLAB_VERSION}" = "X" ]; then
	printf "gmt_mexbuild.sh: MATLAB not found in /Applications - exiting.\n" >&2
	exit -1
fi

# 2. Determine that Xcode CLI has been installed
if ! [ -x "$(command -v xcrun)" ]; then
	echo 'gmt_mexbuild.sh: Error: Xcode CLI tools (xcrun) not found in your search PATH.' >&2
	exit 1
fi

printf "\ngmt_mexbuild.sh: Found most recent MATLAB application: %s\n" ${MATLAB_VERSION} >&2
printf "\ngmt_mexbuild.sh: Will build and place the GMT/MEX toolbox in:\n%s\n" ${BUNDLE_RESOURCES}/bin >&2
# 3. If use does not have write permission, explain they must use sudo
if [ ! -w ${BUNDLE_RESOURCES} ]; then
	printf "You must have sudo privileges on this computer to complete this installation.\n\nContinue? (y/n) [y]:" >&2
	read answer
	if [ "X$answer" = "Xn" ]; then
		exit 0
	fi
fi

printf "\ngmt_mexbuild.sh: Working..." >&2
# 4. Make a listing of all shared libraries but skip symbolic links
mkdir -p /tmp/gmtmexinstall
find ${BUNDLE_RESOURCES}/lib -name '*.dylib' > /tmp/gmtmexinstall/raw.lis
find ${BUNDLE_RESOURCES}/lib -name '*.so'   >> /tmp/gmtmexinstall/raw.lis
while read file; do
	if [ ! -L $file ]; then	# Skipping symbolic links
		echo $file >> /tmp/gmtmexinstall/lib.lis
	fi
done < /tmp/gmtmexinstall/raw.lis
# 5. Make all shared libraries use rpath instead of executable path
#    For each library, replace @executable_path/../lib/*.dylib with @rpath/lib*.dylib
while read file; do
	otool -L $file | grep '@executable_path' | tr '/' ' ' | awk '{printf "install_name_tool -change @executable_path/../lib/%s @rpath/%s %s\n", $4, $4, "'$file'"}' >> /tmp/gmtmexinstall/mexjob.sh
done < /tmp/gmtmexinstall/lib.lis
sh /tmp/gmtmexinstall/mexjob.sh

# 6. Update the plugin to use rpath
install_name_tool -add_rpath ${BUNDLE_RESOURCES}/lib ${BUNDLE_RESOURCES}/lib/gmt/plugins/supplements.so

# 7. Update the gmt executable to use rpath also
install_name_tool -add_rpath ${BUNDLE_RESOURCES}/lib ${BUNDLE_RESOURCES}/bin/gmt

# 8. Build the gmtmex executable
type=$(uname -m)
xcrun clang -I${BUNDLE_RESOURCES}/include/gmt -I/Applications/${MATLAB_VERSION}/extern/include -m64 -fPIC -fno-strict-aliasing -std=c99 -DGMT_MATLAB -c ${BUNDLE_RESOURCES}/share/tools/gmtmex.c -o /tmp/gmtmexinstall/gmtmex.o
xcrun clang -undefined error -arch ${type} -bundle /tmp/gmtmexinstall/gmtmex.o -L${BUNDLE_RESOURCES}/lib -lgmt -L/Applications/${MATLAB_VERSION}/bin/maci64 -lmx -lmex -o ${BUNDLE_RESOURCES}/bin/gmtmex.mexmaci64
cp -f ${BUNDLE_RESOURCES}/share/tools/gmt.m ${BUNDLE_RESOURCES}/bin
# 9. Update the gmtmex plugin to use rpath also
version=$(gmt --version | awk -Fr '{print $1}')	# Skip trailing stuff like rc1
install_name_tool -change @executable_path/../lib/libgmt.${version}.dylib @rpath/libgmt.${version}.dylib ${BUNDLE_RESOURCES}/bin/gmtmex.mexmaci64
install_name_tool -add_rpath ${BUNDLE_RESOURCES}/lib ${BUNDLE_RESOURCES}/bin/gmtmex.mexmaci64
# 10. Clean up and we are done
rm -rf /tmp/gmtmexinstall
printf "done\n" >&2
printf "gmt_mexbuild.sh: You must add this path to your MATLAB path:\n%s\n" ${BUNDLE_RESOURCES}/bin >&2
