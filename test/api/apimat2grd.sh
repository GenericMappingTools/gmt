#!/usr/bin/env bash
#
# Test the C API for passing a global grid as a matrix to grdimage
# and have the central meridian work correctly.
# DVC_TEST

ps=apimat2grd.ps
testapi_grid2matrix
gmt grdimage dump_01d.grd -JQ270/6.5i -Baf -P -K -Cgeo > $ps
gmt grdimage cut_01d.grd -JQ0/6.5i -Baf -O -Y4.25i -Cgeo >> $ps
