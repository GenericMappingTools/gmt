#/bin/sh
# $Id: nightly_gmt_cvsupdate.sh,v 1.4 2008-10-29 23:52:55 guru Exp $
# This script is used to get the latest GMT CVS changes and compile and install everything.
# We do that by first getting the changes and then build and install executables.
# First set some environment parameters since cron will not process login settings
export NETCDFHOME=/sw
export MATLAB=/Applications/MATLAB_R2008a
export GMTHOME=/Users/pwessel/UH/RESEARCH/PROJECTS/GMTdev/GMT
export AWK=awk
export RGBDEF=/usr/X11/share/X11/rgb.txt
export PATH=${PATH}:/sw/bin:$GMTHOME/bin
#-------------------------------------------------------------
cd $GMTHOME
make update
make spotless
make site
