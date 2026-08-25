#!/usr/bin/env bash
#
# Test the C API for reading via a matrix and making a plot with shading
ps=apimatimageshading.ps
testapi_imageshading -I > $ps
