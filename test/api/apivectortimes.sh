#!/usr/bin/env bash
#
# Test the C API for plotting lines with arrows
# See https://github.com/GenericMappingTools/gmt/pull/3528
ps=apivectortimes.ps
testapi_vector_times > $ps
