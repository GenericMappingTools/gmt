#!/bin/awk -f
{
	true_lon = $1
	true_lat = $2
	GMT_lon = $3
	GMT_lat = $4
	true_x = $5
	true_y = $6
	GMT_x = $7
	GMT_y = $8
	
#	Check longitudes & latitudes

	dx = GMT_lon - true_lon
	dy = GMT_lat - true_lat
	if (dy < 0.0) dy = -dy;
	if (dx < 0.0) dx = -dx;
	if (dx > 350) dx -= 360.0;
	if (! (dx == 0.0 || dx == -360.0 || dx = 360.0)) {
		printf "%s: Bad longitude conversion, d = %lg\n", $9, dx
	}
	if (! (dy == 0.0)) {
		printf "%s: Bad latitude conversion, d = %lg\n", $9, dy
	}
	
#	Check x & y

	dx = GMT_x - true_x
	dy = GMT_y - true_y
	if (dy < 0.0) dy = -dy;
	if (dx < 0.0) dx = -dx;
	if (! (dx <= 0.11)) {
		printf "%s: Bad x conversion, d = %lg\n", $9, dx
	}
	if (! (dy <= 0.11)) {
		printf "%s: Bad y conversion, d = %lg\n", $9, dy
	}
}
