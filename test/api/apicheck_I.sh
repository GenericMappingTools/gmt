#!/bin/bash
# Test the C API for i/o involving images
# Note the -W option is not used but must be present.

. ../functions.sh
grep USE_GDAL ../../src/config.mk > tmp
if [ ! -s tmp ]; then
 	header "Test the API for passing IMAGE to psimage (GDAL only)"
	echo "[N/A]"
        rm -f tmp
        exit
fi
# Use another image as test to avoid storing one for the test
cp ../grdimage/gdal/needle.jpg itesti.jpg
# Fake three identical PS files via temporary links
cd orig
ln -s itesti.ps im_f.ps
ln -s itesti.ps im_c.ps
ln -s itesti.ps im_r.ps
cd ..
header "Test the API for passing IMAGE via file (GDAL only)"
ps=im_f.ps
testapi -If -Wf -Ti > $ps
pscmp
header "Test the API for passing IMAGE via copy (GDAL only)"
ps=im_c.ps
testapi -Ic -Wf -Ti > $ps
pscmp
header "Test the API for passing IMAGE via reference (GDAL only)"
ps=im_r.ps
testapi -Ir -Wf -Ti > $ps
pscmp
rm -f itesti.jpg orig/im_[cfr].ps tmp
