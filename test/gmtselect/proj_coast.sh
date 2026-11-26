#!/usr/bin/env bash
# gmtselect -N with a non-cylindrical -J used to crash: dlat was still 0 when the coastline was resampled
cat << EOF > pts.txt
-3	40
20	47
30	50
50	55
-10	45
18	34
EOF
gmt select pts.txt -R-20/60/20/60 -Ns/k -Dl > ref.txt
for proj in -JW20/15c -JL20/40/30/50/10c -JA20/40/10c -JS20/40/10c; do
	gmt select pts.txt -R-20/60/20/60 $proj -Ns/k -Dl > out.txt
	diff out.txt ref.txt --strip-trailing-cr >> fail
done
