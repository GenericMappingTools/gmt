#!/usr/bin/env bash
# Test that an emptied segment is skipped by polygon detection and closure too
cat << EOF > polys.txt
> P1
0 0 1
10 0 1
10 10 1
0 0 1
> P2
20 20 1
30 20 1
30 30 1
EOF
{ printf '1 2\n'; cat polys.txt; } > polys_short.txt

gmt spatial polys_short.txt -Fp -i0:2 > answer1.txt
cat << EOF > truth1.txt
> P1
0	0	1
10	0	1
10	10	1
0	0	1
> P2
20	20	1
30	20	1
30	30	1
20	20	1
EOF
diff truth1.txt answer1.txt --strip-trailing-cr > fail

gmt info polys_short.txt -Fi -i0:2 > answer2.txt
cat << EOF > truth2.txt
1	2	8	0	10
EOF
diff truth2.txt answer2.txt --strip-trailing-cr >> fail

printf '0 0 1\n10 0 1\n10 10 1\n' > tri.txt
printf '30 45\n120 60\n' > short.txt
gmt spatial tri.txt short.txt -Fp -i0:2 -fg > answer3.txt
cat << EOF > truth3.txt
0	0	1
10	0	1
10	10	1
0	0	1
EOF
diff truth3.txt answer3.txt --strip-trailing-cr >> fail
