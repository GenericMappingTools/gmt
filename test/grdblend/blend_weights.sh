#!/usr/bin/env bash
# Ensure weight columns is read OK
ps=blend_weights.ps
cat << EOF > t.lis
ldem_m001_p079_p077_p157.grd 00/078/078/156 1
ldem_m001_p079_p155_p235.grd 00/078/156/234 1
ldem_m001_p079_p233_p313.grd 00/078/234/312 1
ldem_p077_p157_p077_p157.grd 78/156/078/156 1
ldem_p077_p157_p155_p235.grd 78/156/156/234 1
ldem_p077_p157_p233_p313.grd 78/156/234/312 1
EOF
# Make fake tiles
$AWK '{printf "gmt grdmath -R%s/%s/%s/%s -I1 -r X Y MUL = %s\n", $2, $3, $4, $5, $1}' t.lis | sh -s
# Blend tiles
gmt grdblend t.lis  -I1 -R0/156/78/310 -Gt.grd -r
gmt grdimage t.grd -Baf -Jx0.04i -P > $ps
