#!/usr/bin/env bash
#
# Note: Requires coreutils to be installed so grealpath is available
#
# Build include file for cpack to build a complete macOS Bundle.
# List of executables whose shared libraries must also be included
#
# Exceptions:
# For now (6.0.0), need to do a few things manually first, like
# 1. Separate install command to avoid version number in GraphicsMagick directory name
# 2. Build gs from 9.50 tarball and place in /opt (until 9.50 appears in port)

if [ $(which cmake) = "/opt/local/bin/cmake" ]; then
	distro=MacPorts
	top=/opt/local
elif [ $(which cmake) = "/usr/local/bin/cmake" ]; then
	distro=HomeBrew
	top=/usr/local
else
	distro=Fink
	/sw
fi
# 1a. List of executables needed and whose shared libraries also are needed.
#     Use full path if you need something not in your path
EXEPLUSLIBS="/opt/bin/gs /opt/local/bin/gm /opt/local/bin/ffmpeg /opt/local/bin/ogr2ogr /opt/local/bin/gdal_translate /opt/local/lib/libfftw3f_threads.dylib"
# 1b. List of any symbolic links needed
#     Use full path if you need something not in your path
EXELINKS=
# 1c. List of executables whose shared libraries have already been included via other shared libraries
#     Use full path if you need something not in your path
EXEONLY=
# 1d. Shared directories to be added
#     Use full path if you need someting not in your path
EXESHARED="gdal /opt/share/ghostscript /opt/local/lib/proj6/share/proj"
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
