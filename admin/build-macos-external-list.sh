#!/usr/bin/env bash
#
# Note: Requires coreutils to be installed so grealpath is available
#
# Build include file for cpack to build a complete macOS Bundle.
# List of executables whose shared libraries must also be included
#
# Exceptions:
# For now (6.3.0), need to do a few things manually first, like
#   1. Separate install command to avoid version number in GraphicsMagick directory name
#   2. Separate install command to avoid version number in GhostScript directory name
#
# Notes:
#   1. This is tested on macports where gs is a symbolic link to gsc.

GSVERSION=$1	# Get the version of gs from build-release.sh

if [ $(which cmake) = "/opt/local/bin/cmake" ]; then
	distro=MacPorts
	top=/opt/local
elif [ $(which cmake) = "/usr/local/bin/cmake" ]; then
	distro=HomeBrew
	top=/usr/local
else	# Requires either MacPorts of HomeBrew
	exit 1
fi

# Set temporary directory
TMPDIR=${TMPDIR:-/tmp}

# 1a. List of executables needed and whose shared libraries also are needed.
#     Use full path if you need something not in your path
EXEPLUSLIBS="/opt/local/bin/gsc /opt/local/bin/gm /opt/local/bin/ffmpeg /opt/local/bin/ogr2ogr \
 /opt/local/bin/gdal_translate /opt/local/lib/libfftw3f_threads.dylib /opt/local/lib/libomp/libomp.dylib"
# 1b. List of any symbolic links needed
#     Use full path if you need something not in your path
EXELINKS=/opt/local/bin/gs
# 1c. List of executables whose shared libraries have already been included via other shared libraries
#     Use full path if you need something not in your path
EXEONLY=
# 1d. Shared directories to be added (except ghostscript which we do separately)
#     Use full path if you need something not in your path
EXESHARED="gdal /opt/local/lib/proj8/share/proj"
#-----------------------------------------
# 2a. Add the executables to the list given their paths
rm -f ${TMPDIR}/raw.lis
for P in ${EXEONLY} ${EXEPLUSLIBS}; do
	path=$(which $P)
	if [ -L $path ]; then # A symlink
		grealpath $path >> ${TMPDIR}/raw.lis
	else
		echo $path >> ${TMPDIR}/raw.lis
	fi
done
# 2b. Add the symbolic links to the list given their paths as is
for P in $EXELINKS; do
	which $P >> ${TMPDIR}/raw.lis
done
# 2c. Call otool -L recursively to list shared libraries used but exclude system libraries
cc admin/otoolr.c -o build/otoolr
build/otoolr $(pwd) ${EXEPLUSLIBS} >> ${TMPDIR}/raw.lis
# 4. sort into unique list then separate executables from libraries
sort -u ${TMPDIR}/raw.lis > ${TMPDIR}/final.lis
grep dylib ${TMPDIR}/final.lis > ${TMPDIR}/libraries.lis
grep -v dylib ${TMPDIR}/final.lis > ${TMPDIR}/programs.lis
# 5. Build the include file for cpack
cat << EOF
# List of extra executables and shared libraries to include in the macOS installer
# This file was prepared under $distro and used the installation paths of ${USER}.

install (PROGRAMS
EOF
awk '{printf "\t%s\n", $1}' ${TMPDIR}/programs.lis
cat << EOF
	DESTINATION \${GMT_BINDIR}
	COMPONENT Runtime)

install (PROGRAMS
EOF
awk '{printf "\t%s\n", $1}' ${TMPDIR}/libraries.lis
cat << EOF
	DESTINATION \${GMT_LIBDIR}
	COMPONENT Runtime)
EOF

# Optionally add shared resources
if [ ! "X$EXESHARED" = "X" ]; then
	echo ""
	echo "install (DIRECTORY"
fi
for P in $EXESHARED; do
	if [ $P = $(basename $P) ]; then
		echo "	$top/share/$P"
	else
		echo "	$P"
	fi
done
if [ ! "X$EXESHARED" = "X" ]; then
	echo "	DESTINATION share"
	echo "	COMPONENT Runtime)"
fi
cat << EOF

# Place the ghostscript support files while skipping the version directory
install (DIRECTORY
	/opt/local/share/ghostscript/${GSVERSION}/Resource
	/opt/local/share/ghostscript/${GSVERSION}/lib
	/opt/local/share/ghostscript/${GSVERSION}/iccprofiles
	/opt/local/share/ghostscript/fonts
	DESTINATION share/ghostscript
	COMPONENT Runtime)

#
# Place the licenses for runtime dependencies
install (DIRECTORY
	../../admin/Licenses
	DESTINATION share
	COMPONENT Runtime)

# Place the GraphicsMagick config files
install (DIRECTORY
	/opt/local/lib/GraphicsMagick-\${GMT_CONFIG_GM_VERSION}/config
	DESTINATION \${GMT_LIBDIR}/GraphicsMagick
	COMPONENT Runtime)

install (FILES
	/opt/local/share/GraphicsMagick-\${GMT_CONFIG_GM_VERSION}/config/log.mgk
	DESTINATION \${GMT_LIBDIR}/GraphicsMagick/config
	COMPONENT Runtime)
EOF
exit 0
