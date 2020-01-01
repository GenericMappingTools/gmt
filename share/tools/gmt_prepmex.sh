#!/usr/bin/env bash
#
# Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# Until/if we are able to get MATLAB to not override every
# single request for a shared library with its own out-of-date
# version which results in version conflicts, we have to use
# this trick under OS X:
#
# 1. Duplicate the lib, bin, include files from the bundle into
#    an separate directory, here /opt/gmt.
# 2. Rebaptize all libs with unique names by inserting an "X"
# 3. Link the gmt.mex executable with these libraries.
#
# To prepare your system to run the gmt.mex application, run
# /Application/GMT-6.x.x[_r#####.app]/Contents/Resources/share/tools/gmt_prepmex.sh
# This will require sudo privileges.
#
#-------------------------------------------------------------------------
Rel=`gmt --version | awk '{print substr($1,1,3)}'`
printf "\ngmt_prepmex.sh will convert a GMT %s.x bundle so libraries are suitable for building the MATLAB interface.\n" $Rel >&2
printf "You must have sudo privileges on this computer.\n\nContinue? (y/n) [y]:" >&2
read answer
if [ "X$answer" = "Xn" ]; then
	exit 0
fi
here=`pwd`
# First get a reliable absolute path to the bundle's top directory
pushd `dirname $0` > /dev/null
BUNDLEDIR=`pwd | sed -e sB/Contents/Resources/share/toolsBBg`
popd > /dev/null
# Set path to the new gmt installation
MEXGMT5DIR=/tmp/$$/gmt
# Set path to additional subdirectories
MEXLIBDIR=$MEXGMT5DIR/lib
MEXINCDIR=$MEXGMT5DIR/include
MEXSHADIR=$MEXGMT5DIR/share
MEXBINDIR=$MEXGMT5DIR/bin
MEXSUPDIR=$MEXLIBDIR/gmt/plugins
# Create install directory [remove first if exist]
rm -rf $MEXGMT5DIR
printf "gmt_prepmex.sh: Create $MEXGMT5DIR and copy files\n" >&2
mkdir -p $MEXBINDIR $MEXSUPDIR $MEXINCDIR
# Copy the share files
cd $BUNDLEDIR/Contents/Resources
cp -r share $MEXSHADIR
# Copy the include files
cd $BUNDLEDIR/Contents/Resources/include
cp -r gmt $MEXINCDIR
# Copy the bin files
cd $BUNDLEDIR/Contents/Resources/bin
cp -r * $MEXBINDIR
# Now copy the lib files
printf "gmt_prepmex.sh: Copy and rename libraries\n" >&2
cd $BUNDLEDIR/Contents/Resources/lib
# Find a list of all libs shipped with the OSX bundle, except our own:
ls *.dylib | egrep -v 'libgmt.dylib|libpostscriptlight.dylib' > /tmp/l.lis
# For each, duplicate into /opt/gmt but add a leading X to each name
while read lib; do
	new=`echo $lib | awk '{printf "libX%s\n", substr($1,4)}'`
	cp $lib $MEXLIBDIR/$new
done < /tmp/l.lis
# Copy the supplement shared plugin
cp gmt/plugins/supplements.so $MEXLIBDIR/gmt/plugins
cd $MEXLIBDIR
ls *.dylib > /tmp/l.lis
printf "gmt_prepmex.sh: Rebaptize libraries\n" >&2
# For all libs in $MEXLIBDIR, change internal references to contain the leading "X"
while read lib; do
	otool -L $lib | grep executable_path | awk '{print $1}' > /tmp/t.lis
	let k=1
	while read old; do
		new=`echo $old | awk -F/ '{printf "libX%s\n", substr($NF,4)}'`
		if [ $k -eq 1 ]; then # Do the id change
			was=`echo $lib | awk -F/ '{print substr($1,4)}'`
			install_name_tool -id /opt/gmt/lib/$new $lib
		else
			install_name_tool -change $old /opt/gmt/lib/$new $lib
		fi
		let k=k+1
	done < /tmp/t.lis
done < /tmp/l.lis
# Set links to the new libs
ln -s libXgmt.dylib libgmt.dylib
ln -s libXpostscriptlight.dylib libpostscriptlight.dylib
ln -s libXgmt.6.dylib libXgmt.dylib
ln -s libXpostscriptlight.6.dylib libXpostscriptlight.dylib
# If argument gs is given then we also do the same to the GS library.
if [ "$1" == "gs" ]; then
	# Same stuff for gs which is called by psconvert as a system call.
	# Here we must determine from where to copy...
	GSV=`gs --version`
	if [ -d /sw/lib ]; then			# Fink has no shared lib yet...
		FROM=/sw/lib
		echo "Sorry, no libgs.dylib under fink yet"
	elif [ -d /opt/local/lib ]; then	# Macports
		FROM=/opt/local/lib
		cp $FROM/libgs.${GSV}.dylib libXgs.${GSV}.dylib 
		cp $FROM/libfreetype.6.dylib libXfreetype.6.dylib
		install_name_tool -id /opt/gmt/lib/libXgs.${GSV}.dylib libXgs.${GSV}.dylib 
		install_name_tool -id /opt/gmt/lib/libXfreetype.6.dylib libXfreetype.6.dylib
		install_name_tool -change $FROM/libtiff.5.dylib /opt/gmt/lib/libXtiff.5.dylib libXgs.${GSV}.dylib 
		install_name_tool -change $FROM/libfreetype.6.dylib /opt/gmt/lib/libXfreetype.6.dylib libXgs.${GSV}.dylib 
	elif [ -d /usr/local/lib ]; then		# Brew
		FROM=/usr/local/lib
		echo "Sorry, no libgs.dylib under HomeBrew yet"
	fi
fi

# Do plugin supplement separately since not called lib*
cd gmt/plugins
otool -L supplements.so | grep executable_path | awk '{print $1}' > /tmp/t.lis
let k=1
while read old; do
	new=`echo $old | awk -F/ '{printf "libX%s\n", substr($NF,4)}'`
	install_name_tool -change $old /opt/gmt/lib/$new supplements.so
	let k=k+1
done < /tmp/t.lis

# Do bin dir
cd $MEXBINDIR
otool -L gmt | grep executable_path | awk '{print $1}' > /tmp/t.lis
let k=1
while read old; do
	new=`echo $old | awk -F/ '{printf "libX%s\n", substr($NF,4)}'`
	install_name_tool -change $old /opt/gmt/lib/$new gmt
	let k=k+1
done < /tmp/t.lis
chmod -R ugo+r $MEXGMT5DIR
printf "gmt_prepmex.sh: Install /opt/gmt\n" >&2
sudo cp -fpR $MEXGMT5DIR /opt
rm -rf /tmp/$$
cd $here
version=`/opt/gmt/bin/gmt-config --version`
# Report
cat << EOF >&2
gmt_prepmex.sh: Made updated GMT $version installation in /opt/gmt
gmt_prepmex.sh: Add /opt/gmt to your .gmtversions and run gmtswitch to select this version
gmt_prepmex.sh: MATLAB may need a gmt.conf file with GMT_CUSTOM_LIBS=/opt/gmt/lib/gmt/plugins/supplements.so in the startup directory
EOF
