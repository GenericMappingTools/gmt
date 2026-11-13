#!/usr/bin/env bash
#
# Tests mapproject precision of the van der Grinten projection close to the origin, the central meridian,
# the equator and the poles, where Snyder's formulas subtract large terms.
# Expected values are from 50-digit computations of Snyder's (29-1) to (29-6) on a sphere of radius 6371000 m.
# Forward results must be within 1e-6 m in x and y; inverse results within 1e-6 m on the sphere.

R=6371000
OPT="-Rg -C -F --PROJ_ELLIPSOID=$R --PROJ_SCALE_FACTOR=1 --FORMAT_FLOAT_OUT=%.17g"

fwd () {	# name lon lat x y
	echo "$2 $3" | gmt mapproject -Jv0/1:1 $OPT | $AWK -v n="$1" -v x="$4" -v y="$5" '{dx = $1 - x; dy = $2 - y; if (dx < 0) dx = -dx; if (dy < 0) dy = -dy;
		if (!(dx <= 1e-6 && dy <= 1e-6)) printf "%s: forward off by (%g, %g) m\n", n, dx, dy}'
}
inv () {	# name x y lon lat
	echo "$2 $3" | gmt mapproject -Jv0/1:1 $OPT -I | $AWK -v n="$1" -v lon="$4" -v lat="$5" -v R=$R '{d2r = 3.14159265358979323846 / 180;
		dl = $1 - lon; while (dl > 180) dl -= 360; while (dl < -180) dl += 360;
		ex = dl * cos(lat * d2r) * d2r * R; ey = ($2 - lat) * d2r * R; d = sqrt(ex * ex + ey * ey);
		if (!(d <= 1e-6)) printf "%s: inverse off by %g m\n", n, d}'
}

: > fail
fwd near_origin_1 -1.988028369930364e-10 -0.04625606949868413 -2.2105865416348358e-5 -5143.4405944331969 >> fail
fwd near_origin_2 3e-07 2e-07 0.03335847799336762 0.022238985328911746 >> fail
fwd near_central_meridian 1e-12 45 1.032114901017277e-7 5363026.3434325397 >> fail
fwd near_pole_1 10 89.999 10423.803467322525 19921339.853950618 >> fail
fwd near_pole_2 0 89.9999999995 0 20015020.07952856 >> fail
fwd near_pole_3 150 89.99 387999.60355825409 19940052.131019978 >> fail
fwd near_equator -120 -1e-09 -13343391.197347048 -0.0001111949266448333 >> fail
fwd general 100 0.5 11119426.987837261 55645.558358831678 >> fail
inv near_origin_2 0.03335847799336762 0.022238985328911746 3e-07 2e-07 >> fail
inv general 11119426.987837261 55645.558358831678 100 0.5 >> fail
