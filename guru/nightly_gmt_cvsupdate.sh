#/bin/sh
# $Id: nightly_gmt_cvsupdate.sh,v 1.1 2008-08-25 00:47:58 guru Exp $
# This script is used to get the latest GMT CVS changes and compile and install everything.
# We do that by first getting the changes and then build executables and documentation.
# PS: For some reason make site crashes in latex.  Until I can figure it out we just update executables
cd /Users/pwessel/UH/RESEARCH/PROJECTS/GMTdev/GMT
make update
make install
