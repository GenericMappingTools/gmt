#!/bin/csh
#
#	$Id: shrink_job.csh,v 1.1 2004-09-05 04:19:26 pwessel Exp $

set VERSION = "v4.0"
polygon_shrink res_f/${VERSION}_final_dbase.b 0.2 res_h/${VERSION}_final_dbase_0.2km.b
polygon_shrink res_f/${VERSION}_final_dbase.b 1 res_i/${VERSION}_final_dbase_1km.b
polygon_shrink res_f/${VERSION}_final_dbase.b 5 res_l/${VERSION}_final_dbase_5km.b
polygon_shrink res_f/${VERSION}_final_dbase.b 25 res_c/${VERSION}_final_dbase_25km.b
