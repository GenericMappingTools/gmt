#!/usr/bin/env bash
#
# Note: Requires coreutils to be installed so grealpath is available
#
# Build include file for cpack to build a complete macOS Bundle.
# List of executables whose shared libraries must also be included

if [ `which cmake` = "/opt/local/bin/cmake" ]; then
	distro=MacPorts
elif [ `which cmake` = "/usr/local/bin/cmake" ]; then
	distro=HomeBrew
else
	distro=Fink
fi
# 1a. List of executables whose shared libraries also are needed.
EXEPLUSLIBS="gsc gm ffmpeg"
# 1b. List of symbolic links.
EXELINKS="gs"
# 1c. List of executables whose shared libraries have been included via GDAL
EXEONLY="ogr2ogr gdal_translate"
#-----------------------------------------
# 2a. Add the executables to the list given their paths
rm -f /tmp/raw.lis
for P in ${EXEONLY} ${EXEPLUSLIBS}; do
	path=`which $P`
	if [ -L $path ]; then # A symlink
		grealpath $path >> /tmp/raw.lis
	else
		echo $path >> /tmp/raw.lis
	fi
done
# 2b. Add the symbolic links to the list given their paths as is
rm -f /tmp/raw.lis
for P in $EXELINKS; do
	which $P >> /tmp/raw.lis
done
# 2c. Use otools -L to list shared libraries used but exclude system libraries
for P in $EXEPLUSLIBS; do
	path=`which $P`
	otool -L $path | egrep -v '/usr/lib|/System/Library' | tr ':' ' ' | awk '{print $1}' > /tmp/dump.lis
	while read file; do
		if [ -L $file ]; then # A symlink
			grealpath $file >> /tmp/raw.lis
		else
			echo $file >> /tmp/raw.lis
		fi
	done < /tmp/dump.lis
done
# 4. sort into unique list then split executables from libraries
sort -u /tmp/raw.lis > /tmp/final.lis
grep dylib /tmp/final.lis > /tmp/libraries.lis
grep -v dylib /tmp/final.lis > /tmp/programs.lis

# 5. Build the include file for cpack
cat << EOF
# List of extra executables and shared libraries to include in the macOS installer
# This file was prepared under $distro and used the installation paths of ${USER}.

install (PROGRAMS
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
