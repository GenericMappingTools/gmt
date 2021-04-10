#!/usr/bin/env bash
#
# Compile and link the gmtmex shared library for the user's (most recent) MATLAB version
MATLAB=$(ls /Applications | grep MATLAB | sort -r | head -1)
if [ "X${MATLAB}" = "X" ]; then
	printf "MATLAB not found in /Applications - exiting.\n" >&2
	exit -1
fi

printf "\ngmt_mexbuild.sh found most recent MATLAB application: %s\n" ${MATLAB} >&2
printf "\ngmt_mexbuild.sh will build and place the GMT/MEX toolbox in %s.\n" ${BUNDLE_RESOURCES}/bin >&2
printf "You must have sudo privileges on this computer.\n\nContinue? (y/n) [y]:" >&2
read answer
if [ "X$answer" = "Xn" ]; then
	exit 0
fi

type=$(uname -m)
xcrun clang -I${BUNDLE_RESOURCES}/include/gmt -I/Applications/${MATLAB}/extern/include -m64 -fPIC -fno-strict-aliasing -std=c99 -DGMT_MATLAB -c ${BUNDLE_RESOURCES}/share/tools/gmtmex.c -o /tmp/gmtmex.o
xcrun clang -undefined error -arch ${type} -bundle -DGMT_MATLAB /tmp/gmtmex.o -${BUNDLE_RESOURCES}/lib -lgmt -L/Applications/${MATLAB}/bin/maci64 -lmx -lmex -o ${BUNDLE_RESOURCES}/bin/gmtmex.mexmaci64
cp -f ${BUNDLE_RESOURCES}/share/tools/gmt.m ${BUNDLE_RESOURCES}/bin
printf "You must add this path to your MATLAB path: %s\n" ${BUNDLE_RESOURCES}/bin >&2
