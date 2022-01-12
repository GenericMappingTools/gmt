#!/usr/bin/env bash
#
# Test the C API for plotting lines with arrows
# See https://github.com/GenericMappingTools/gmt/pull/3528
# DVC_TEST
ps=apivectorplot.ps
testapi_vector_plot > $ps
