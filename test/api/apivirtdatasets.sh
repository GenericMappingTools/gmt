#!/usr/bin/env bash
#
# Originated via https://github.com/GenericMappingTools/gmt/issues/5890
#
# Test case where we pass two separate datasets as virtual files
# to a module.  The API used to only allow one virtual dataset
# when read as dataset (but many if read rec-by-rec).
# Create two crossing lines:
gmt project -C22/49 -E-90/-20 -G10 -Q > I1.txt
gmt project -C0/-60 -E-90/-10 -G10 -Q > I2.txt
# Compute the crossing on command line:
gmt spatial I1.txt I2.txt -Fl -Ie > answer.txt
# Same but via API and virtual files via GMT_IS_REFERENCE:
testapi_spatial result_ref.txt
diff -q --strip-trailing-cr result_ref.txt answer.txt > fail
# Same but via API and virtual files via GMT_IS_DUPLICATE:
testapi_spatial result_dup.txt -duplicate
diff -q --strip-trailing-cr result_dup.txt answer.txt >> fail
