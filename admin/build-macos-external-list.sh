#!/bin/bash
# Build include file for cpack to build complete macOS Bundle
# List of executables whose shared libraries must also be included

EXE="gs gm ffmpeg"
# List of executables whose shared libraries have been included via GDAL
which ogr2ogr > /tmp/raw.lis
which gdal_translate >> /tmp/raw.lis
for P in $EXE; do
	path=`which $P`
	echo $path >> /tmp/raw.lis
	otool -L $path | egrep -v '/usr/lib|/System/Library' | tr ':' ' ' | awk '{print $1}' >> /tmp/raw.lis
done
sort -u /tmp/raw.lis > /tmp/unique.lis
grep dylib /tmp/unique.lis > /tmp/libraries.lis
grep -v dylib /tmp/unique.lis > /tmp/programs.lis

cat << EOF
# List of extra executables and shared libraries to include in the macOS installer
# This file is prepared to use ${USER} installation paths.

install (PROGRAMS)
EOF
awk '{printf "\t%s\n", $1}' /tmp/programs.lis
cat << EOF
	DESTINATION \${GMT_BINDIR}
	COMPONENT Runtime)

install (PROGRAMS
EOF
awk '{printf "\t%s\n", $1}' /tmp/libraries.lis
cat << EOF
	DESTINATION \${GMT_LIBDIR}
	COMPONENT Runtime)
EOF
