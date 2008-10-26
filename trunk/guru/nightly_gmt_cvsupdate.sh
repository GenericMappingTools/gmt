#/bin/sh
# $Id: nightly_gmt_cvsupdate.sh,v 1.3 2008-10-26 01:26:37 guru Exp $
# This script is used to get the latest GMT CVS changes and compile and install everything.
# We do that by first getting the changes and then build and install executables.
cd /Users/pwessel/UH/RESEARCH/PROJECTS/GMTdev/GMT
make update
make spotless
make site
