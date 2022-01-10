#!/usr/bin/env bash
#
# Test the C API for passing a global grid as a matrix to grdimage
# and have the central meridian work correctly.
# DVC_TEST

ps=apimat_360.ps
testapi_matrix_360 > $ps
