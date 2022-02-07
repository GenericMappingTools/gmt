#!/usr/bin/env bash
#
# Test the C API for passing a global grid as a matrix to grdimage via reference
# and have the central meridian work correctly.

ps=apimat_360_ref.ps
testapi_matrix_360_ref > $ps
