#!/bin/csh
#
#	$Id$
#
#	Calculates the node-grids for each resolution

set VERSION = "v4.1"
set d = (_25km _5km _1km _0.2km "")
set dx = (20 10 5 2 1) 
set i = 1
foreach res (c l i h f)

	set this = $d[$i]
	polygon_setnodes res_${res}/${VERSION}_final_dbase${this}.b $dx[$i] res_${res}/${VERSION}_final_nodes${this}.grd
	@ i++
end
