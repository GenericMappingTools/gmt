#!/usr/bin/env bash
#
# Tests mapproject precision of spherical projections close to the poles, the equator and the projection
# centers, where asin/acos of values close to 1, slowly converging iterations and cancellation lost digits.
# Expected values are from 50-digit computations of the same formulas on a sphere of radius 6371000 m.
# Forward results must be within 1e-6 m in x and y; inverse results within 1e-6 m on the sphere.
# Each case also lists the error of GMT before these fixes at that point.

R=6371000
OPT="-Rg -C -F --PROJ_ELLIPSOID=$R --PROJ_SCALE_FACTOR=1 --FORMAT_FLOAT_OUT=%.17g"

fwd () {	# name J lon lat x y
	echo "$3 $4" | gmt mapproject $2 $OPT | $AWK -v n="$1" -v x="$5" -v y="$6" '{dx = $1 - x; dy = $2 - y; if (dx < 0) dx = -dx; if (dy < 0) dy = -dy;
		if (!(dx <= 1e-6 && dy <= 1e-6)) printf "%s: forward off by (%g, %g) m\n", n, dx, dy}'
}
inv () {	# name J x y lon lat
	echo "$3 $4" | gmt mapproject $2 $OPT -I | $AWK -v n="$1" -v lon="$5" -v lat="$6" -v R=$R '{d2r = 3.14159265358979323846 / 180;
		dl = $1 - lon; while (dl > 180) dl -= 360; while (dl < -180) dl += 360;
		ex = dl * cos(lat * d2r) * d2r * R; ey = ($2 - lat) * d2r * R; d = sqrt(ex * ex + ey * ey);
		if (!(d <= 1e-6)) printf "%s: inverse off by %g m\n", n, d}'
}

: > fail
# stere lat0=45 inverse near pole: was off by 12 cm
inv stere_lat0=45_inverse_near_pole -Js0/45/1:1 -0.017357256339947585 5277909.214056263 -97.542694822582529 89.999999865599477 >> fail
# ortho lat0=45 inverse near pole: was off by 10 cm
inv ortho_lat0=45_inverse_near_pole -Jg0/45/1:1 0.08870776260249785 4504977.271652754 63.490652009383727 89.999999108501115 >> fail
# laea lat0=45 inverse near pole: was off by 11 cm
inv laea_lat0=45_inverse_near_pole -Ja0/45/1:1 -0.10579423104278692 4876152.250180231 -63.503398847417915 89.999999017826227 >> fail
# aeqd lat0=45 inverse near pole: was off by 11 cm
inv aeqd_lat0=45_inverse_near_pole -Je0/45/1:1 0.10233602668431892 5003771.636041093 55.651667040360727 89.999998996408179 >> fail
# gnom lat0=45 inverse near pole: was off by 13 cm
inv gnom_lat0=45_inverse_near_pole -Jf0/45/60/1:1 0.15308872458159825 6371000.127803413 120.55401575168985 89.999998869516376 >> fail
# stere lat0=90 inverse near pole: was off by 7 cm
inv stere_lat0=90_inverse_near_pole -Js0/90/1:1 0.04133658205716969 -0.05235842355927538 38.290826877210778 89.99999940006974 >> fail
# aeqd lat0=30 forward near center: was off by 5 cm
fwd aeqd_lat0=30_forward_near_center -Je0/30/1:1 1.4827936074599344e-07 29.9999995751583 0.014278951263658303 -0.047240241810608076 >> fail
# stere lat0=0 forward: was off by 210 m
fwd stere_lat0=0_forward -Js0/0/1:1 -75.70559764955098 -79.60853200671201 -2132195.793523974 -11998652.50824588 >> fail
# moll forward near pole: was off by 28 m
fwd moll_forward_near_pole -Jw0/1:1 149.36945965937616 89.99999892312347 111.64290700674565 9009954.605627874 >> fail
# eck4 forward near pole: was off by 2 cm
fwd eck4_forward_near_pole -Jkf0/1:1 166.55853481804303 -89.99999885355678 7820047.623256444 -8451134.227915678 >> fail
# moll inverse near pole: was off by 4 cm
inv moll_inverse_near_pole -Jw0/1:1 1.0010467571936355 9009954.605574686 1.2166560791066358 89.999998756225293 >> fail
# vandg forward near origin: was off by 16189 km
fwd vandg_forward_near_origin -Jv0/1:1 -1.988028369930364e-10 -0.04625606949868413 -2.2105865416348358e-05 -5143.440594433197 >> fail
# wintri forward near origin: was off by 6 cm
fwd wintri_forward_near_origin -Jr0/1:1 1.1591390849393652e-06 -1.1165202501808802e-07 0.10546629962186516 -0.012415138731602734 >> fail
