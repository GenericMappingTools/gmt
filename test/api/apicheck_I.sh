#!/bin/bash
#	$Id$
#
# Test the C API for i/o involving images
# Note the -W option is not used but must be present.
# Requires the GDAL build
# The comparison PS was created thus:
# psimage itesti.jpg -W6i -F0.25p -P --PS_MEDIA=letter --PS_CHAR_ENCODING=Standard+ > apicheck_I.ps

# Only do this when GDAL is installed
GDAL=`grdreformat 2>&1 | grep -c gd`
if [ $GDAL -eq 0 ]; then exit; fi

# Use another image as test to avoid storing one for the test
ln -fs "${src:=.}"/../grdimage/gdal/needle.jpg itesti.jpg
ps=apicheck_I_f.ps
testapi -If -Wf -Ti > $ps
ps=apicheck_I_c.ps
testapi -Ic -Wf -Ti > $ps
ps=apicheck_I_r.ps
testapi -Ir -Wf -Ti > $ps
psref=apicheck_I.ps
