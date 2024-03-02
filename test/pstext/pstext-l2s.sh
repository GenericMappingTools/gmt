#!/usr/bin/env bash
#
# Test pstext longopts translation.

m=pstext
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A
--l2stranstest -C5/10+to
--l2stranstest -Dj1/2+v2p,red -DJ
--l2stranstest -F+a10 -F+A20
--l2stranstest -F+A30+cTL+f12,Helvetica,red
--l2stranstest -F+cCM+h+jTL
--l2stranstest -F+l -F+r -F+r10
--l2stranstest -F+tblab -F+z%4.2f -F+z
--l2stranstest -Gred+n -Ggreen+n
--l2stranstest -L -L
--l2stranstest -M
--l2stranstest -N -N
--l2stranstest -Ql -Qu
--l2stranstest -S20/30/blue
--l2stranstest -Z -Z
--l2stranstest -W1p,blue,solid
EOF

# module-specific longopts
gmt $m $l2s --azimuth >> $b
gmt $m $l2s --clearance=5/10+textbox:o >> $b
gmt $m $l2s --offset=away:1/2+line:2p,red --offset=corners >> $b
gmt $m $l2s --attributes+angle:10 --attrib+zerocenter:20 >> $b
gmt $m $l2s --attrib+Angle:30+rjustify:TL+font:12,Helvetica,red >> $b
gmt $m $l2s --attrib+region_justify:CM+header+justify:TL >> $b
gmt $m $l2s --attrib+label --attrib+record --attrib+rec_number:10 >> $b
gmt $m $l2s --attrib+text:blab --attrib+zformat:'%4.2f' --attrib+zvalues >> $b
gmt $m $l2s --fill=red+no_text --fill=green+C >> $b
gmt $m $l2s --listfonts --list >> $b
gmt $m $l2s --paragraph >> $b
gmt $m $l2s --noclip --no_clip >> $b
gmt $m $l2s --case=lower --change_case=upper >> $b
gmt $m $l2s --shade=20/30/blue >> $b
gmt $m $l2s --zvalues --threeD >> $b
gmt $m $l2s --pen=1p,blue,solid >> $b

diff $a $b --strip-trailing-cr > fail
