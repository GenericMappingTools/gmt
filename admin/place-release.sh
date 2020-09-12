#!/usr/bin/env bash
#
# Script that places the GMT release files on the SOEST GMT ftp server.
#

if [ ! "${USER}" = "pwessel" ]; then	# Place file in pwessel SOEST ftp release directory and set permissions
	echo "place-release.sh: Can currently only be run by user pwessel" >&2
	exit 1
fi

TOPDIR=$(pwd)

if [ $# -gt 0 ]; then
	cat <<- EOF  >&2
	Usage: place-release.sh
	
	place-release.sh must be run from top-level gmt directory.
	Typically run after build-release.sh has been done and that the
	Windows installer files have been placed in pwessel staging directory.
	EOF
	exit 1
fi
if [ ! -d cmake ]; then
	echo "place-release.sh: Must be run from top-level gmt directory" >&2
	exit 1
fi

# 1. Get the version string
Version=$(src/gmt --version)
# 2. Build the release.sh script
cat << EOF > /tmp/release.sh
#!/bin/bash
# Script to be placed in pwessel ftp/release directory and executed
# Place macOS bundle with read permissions
if [ -f gmt-${Version}-darwin-x86_64.dmg ]; then
	cp -f gmt-${Version}-darwin-x86_64.dmg ../../gmt/bin
	chmod og+r ../../gmt/bin/gmt-${Version}-darwin-x86_64.dmg
fi
# Place Windows 32-bit installer with read and execute permissions
if [ -f gmt-${Version}-win32.exe ]; then
	cp -f gmt-${Version}-win32.exe ../../gmt/bin
	chmod og+rx ../../gmt/bin/gmt-${Version}-win32.exe
fi
# Place Windows 64-bit installer with read and execute permissions
if [ -f gmt-${Version}-win64.exe ]; then
	cp -f gmt-${Version}-win64.exe ../../gmt/bin
	chmod og+rx ../../gmt/bin/gmt-${Version}-win64.exe
fi
# Place tar balls with read permissions
if [ -f gmt-${Version}-src.tar.gz ]; then
	cp -f gmt-${Version}-src.tar.gz ../../gmt
	chmod og+r ../../gmt/gmt-${Version}-src.tar.gz
fi
# Place tar balls with read permissions
if [ -f gmt-${Version}-src.tar.xz ]; then
	cp -f gmt-${Version}-src.tar.xz ../../gmt
	chmod og+r ../../gmt/gmt-${Version}-src.tar.xz
fi
# Self-destruct
rm -f release.sh
EOF
# 3. Copy script to pwessel/release dir:
scp /tmp/release.sh ftp.soest.hawaii.edu:/export/ftp1/ftp/pub/pwessel/release
# 4. Run the release.sh script
ssh pwessel@ftp.soest.hawaii.edu 'bash /export/ftp1/ftp/pub/pwessel/release/release.sh'
# 5. Remove the local script copy
rm -f /tmp/release.sh
