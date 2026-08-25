#!/usr/bin/env bash
# 
# Purpose: This is a CLI version of what testapi_modern.c should produce
# Fails until we fix the page size and orientation from the C program.

# This is the fixed baseline DVC plot we will compare with
#gmt begin apimodern ps
#	gmt set PS_MEDIA letter
#	gmt basemap -BWESN -Bafg -JM7.27/42.27/16.25c -R5.5/41.425/9.0/43.1+r
#gmt end show

# Run the C code to produce apimodern.ps
testapi_modern
ps=apimodern.ps
