#!/bin/csh
#
#	$Id: bin_job.csh,v 1.1 2004-09-05 04:19:26 pwessel Exp $

set VERSION = "v4.0"

touch polygon_to_bins.c; \rm polygon_to_bins
make polygon_to_bins DEGREES=1
polygon_to_bins res_f/${VERSION}_final_dbase.b res_f/${VERSION}_final_nodes.grd res_f/binned_GSHHS_f

touch polygon_to_bins.c; \rm polygon_to_bins
make polygon_to_bins DEGREES=2
polygon_to_bins res_h/${VERSION}_final_dbase_0.2km.b res_h/${VERSION}_final_nodes_0.2km.grd res_h/binned_GSHHS_h

touch polygon_to_bins.c; \rm polygon_to_bins
make polygon_to_bins DEGREES=5 
polygon_to_bins res_i/${VERSION}_final_dbase_1km.b res_i/${VERSION}_final_nodes_1km.grd res_i/binned_GSHHS_i

touch polygon_to_bins.c; \rm polygon_to_bins
make polygon_to_bins DEGREES=10
polygon_to_bins res_l/${VERSION}_final_dbase_5km.b res_l/${VERSION}_final_nodes_5km.grd res_l/binned_GSHHS_l

touch polygon_to_bins.c; \rm polygon_to_bins
make polygon_to_bins DEGREES=20
polygon_to_bins res_c/${VERSION}_final_dbase_25km.b res_c/${VERSION}_final_nodes_25km.grd res_c/binned_GSHHS_c
#
shoremaker res_f/binned_GSHHS_f
shoremaker res_h/binned_GSHHS_h
shoremaker res_i/binned_GSHHS_i
shoremaker res_l/binned_GSHHS_l
shoremaker res_c/binned_GSHHS_c
rm res_?/binned_GSHHS_?.bin
rm res_?/binned_GSHHS_?.seg
rm res_?/binned_GSHHS_?.pt
