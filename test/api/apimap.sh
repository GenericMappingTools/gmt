#!/usr/bin/env bash
#
# Test the C API for simulating a modern mode basemap plot
# See https://github.com/GenericMappingTools/gmt/issues/4518
ps=apimap.ps
testapi_map > $ps
