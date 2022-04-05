#!/usr/bin/env bash
#
# Test the C API for plotting datetimes
# See https://github.com/GenericMappingTools/gmt/pull/6521
ps=apivectortimes.ps
testapi_vector_times > $ps
