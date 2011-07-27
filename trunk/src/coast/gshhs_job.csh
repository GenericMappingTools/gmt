#!/bin/csh
#
#	$Id$

set VERSION = v4.1

polygon_to_gshhs res_f/${VERSION}_final_dbase.b > gshhs_f.b
polygon_to_gshhs res_h/${VERSION}_final_dbase_0.2km.b > gshhs_h.b
polygon_to_gshhs res_i/${VERSION}_final_dbase_1km.b > gshhs_i.b
polygon_to_gshhs res_l/${VERSION}_final_dbase_5km.b > gshhs_l.b
polygon_to_gshhs res_c/${VERSION}_final_dbase_25km.b > gshhs_c.b
