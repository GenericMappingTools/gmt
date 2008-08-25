#/bin/sh
# $Id: nightly_gmt_cvsupdate.sh,v 1.2 2008-08-25 00:48:43 guru Exp $
# This script is used to get the latest GMT CVS changes and compile and install everything.
# We do that by first getting the changes and then build and install executables.
cd /Users/pwessel/UH/RESEARCH/PROJECTS/GMTdev/GMT
make update
make install
