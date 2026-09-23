#!/usr/bin/env bash
#
# Test fractional time zones in +z<TZ> (issue #9220). At lon = 0 the offset
# in minutes is recovered from the -C output as SolarNoon*1440 - 720 + EQ_time.

cat << EOF > tz.txt
08 480
08:30 510
-08:30 -510
-00:30 -30
08:30:45 510.75
EOF

while read tz minutes; do
	gmt solar -I0/0+d2020-01-01T00:00:00+z$tz -C -o6,9 | awk -v tz=$tz -v m=$minutes \
		'{got = $1 * 1440 - 720 + $2; if (got - m > 1e-4 || m - got > 1e-4) printf "+z%s gave %.6f minutes, expected %s\n", tz, got, m}'
done < tz.txt > fail

gmt solar -I0/0+zabc -C > /dev/null 2>&1 && echo "+zabc was accepted" >> fail
