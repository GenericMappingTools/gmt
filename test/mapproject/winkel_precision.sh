#!/usr/bin/env bash
#
# Tests mapproject precision of the Winkel Tripel projection close to the origin and the poles,
# where acos() of values close to 1 and sqrt(1 - X * X) with X close to 1 lost digits, and where the
# inverse iteration did not converge from its initial guess.
# Expected values are from 50-digit computations of the Aitoff and equirectangular average on a sphere
# of radius 6371000 m with standard parallel 50d28'.
# Forward results must be within 1e-6 m in x and y; inverse results within 1e-6 m on the sphere.

R=6371000
OPT="-Rg -C -F --PROJ_ELLIPSOID=$R --PROJ_SCALE_FACTOR=1 --FORMAT_FLOAT_OUT=%.17g"

fwd () {	# name lon lat x y
	echo "$2 $3" | gmt mapproject -Jr0/1:1 $OPT | $AWK -v n="$1" -v x="$4" -v y="$5" '{dx = $1 - x; dy = $2 - y; if (dx < 0) dx = -dx; if (dy < 0) dy = -dy;
		if (!(dx <= 1e-6 && dy <= 1e-6)) printf "%s: forward off by (%g, %g) m\n", n, dx, dy}'
}
inv () {	# name x y lon lat
	echo "$2 $3" | gmt mapproject -Jr0/1:1 $OPT -I | $AWK -v n="$1" -v lon="$4" -v lat="$5" -v R=$R '{d2r = 3.14159265358979323846 / 180;
		dl = $1 - lon; while (dl > 180) dl -= 360; while (dl < -180) dl += 360;
		ex = dl * cos(lat * d2r) * d2r * R; ey = ($2 - lat) * d2r * R; d = sqrt(ex * ex + ey * ey);
		if (!(d <= 1e-6)) printf "%s: inverse off by %g m\n", n, d}'
}

: > fail
fwd near_origin 1.1710159982592087e-06 4.496557511326608e-07 0.10654694138008786 0.049999438262500175 >> fail
fwd near_pole -22.406925872937023 -89.99999633867975 -792965.2762461863 -10007542.99476921 >> fail
inv near_origin_1 -0.0727812131169213 0.0016451217669125457 -7.999100098856121e-07 1.4794935493516501e-08 >> fail
inv near_origin_2 -0.0051468098261375095 0.004792666560499058 -5.6566585284737996e-08 4.3101485878210115e-08 >> fail
inv near_origin_3 0.027296025384403833 -0.022238985328911748 3e-07 -2e-07 >> fail
inv moderate 45493.199223794814 27798.775765065355 0.5 0.25 >> fail
