#!/bin/csh
#
#	$Id$

set VERSION = "v4.1"

polygon_to_bins res_f/${VERSION}_final_dbase.b 1 res_f/${VERSION}_final_nodes.grd res_f/binned_GSHHS_f
polygon_to_bins res_h/${VERSION}_final_dbase_0.2km.b 2 res_h/${VERSION}_final_nodes_0.2km.grd res_h/binned_GSHHS_h
polygon_to_bins res_i/${VERSION}_final_dbase_1km.b 5 res_i/${VERSION}_final_nodes_1km.grd res_i/binned_GSHHS_i
polygon_to_bins res_l/${VERSION}_final_dbase_5km.b 10 res_l/${VERSION}_final_nodes_5km.grd res_l/binned_GSHHS_l
polygon_to_bins res_c/${VERSION}_final_dbase_25km.b 20 res_c/${VERSION}_final_nodes_25km.grd res_c/binned_GSHHS_c
#
shoremaker res_f/binned_GSHHS_f
shoremaker res_h/binned_GSHHS_h
shoremaker res_i/binned_GSHHS_i
shoremaker res_l/binned_GSHHS_l
shoremaker res_c/binned_GSHHS_c
rm res_?/binned_GSHHS_?.bin
rm res_?/binned_GSHHS_?.seg
rm res_?/binned_GSHHS_?.pt
