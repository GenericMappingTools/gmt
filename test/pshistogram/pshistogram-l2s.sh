#!/usr/bin/env bash
#
# Test pshistogram longopts translation.

m=pshistogram
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A
--l2stranstest -Cthis/file -Cthat/file+b
--l2stranstest -D+b+fTimes+o2p+r
--l2stranstest -E1i -E8p+o2p
--l2stranstest -F
--l2stranstest -Gred -Gp25+bgreen+fgray
--l2stranstest -GP3+bwhite+fred+r300
--l2stranstest -Io -Io -IO
--l2stranstest -Lb -Ll -Ll
--l2stranstest -Lh -Lh
--l2stranstest -N0 -N1
--l2stranstest -N2 -N2+p3p
--l2stranstest -Qr
--l2stranstest -S
--l2stranstest -T100/500/10+n -T90/20/30+i
--l2stranstest -Tfile
--l2stranstest -W2p,red
--l2stranstest -Z0 -Z1 -Z2+w
--l2stranstest -Z3 -Z4 -Z5
EOF

# module-specific longopts
gmt $m $l2s --horizontal >> $b
gmt $m $l2s --cpt=this/file --cmap=that/file+bin >> $b
gmt $m $l2s --annotate+beneath+font:Times+offest:2p+rotate >> $b
gmt $m $l2s --barwidth=1i --width=8p+offset:2p >> $b
gmt $m $l2s --center >> $b
gmt $m $l2s --fill=red --fill=bit:25+bg:green+foreground:gray >> $b
gmt $m $l2s --fill=bitreverse:3+background:white+fg:red+dpi:300 >> $b
gmt $m $l2s --inquire=nonzero --inquire=no_zero --inquire=all >> $b
gmt $m $l2s --extreme=both --out_range=low --extreme=first >> $b
gmt $m $l2s --extreme=high --extreme=last >> $b
gmt $m $l2s --distribution=meanstddev --distribution=medianL1 >> $b
gmt $m $l2s --distribution=LMSscale --distribution=lmsscale+pen:3p >> $b
gmt $m $l2s --cumulative=reverse >> $b
gmt $m $l2s --stairs >> $b
gmt $m $l2s --range=100/500/10+number --bin=90/20/30+inverse >> $b
gmt $m $l2s --series=file >> $b
gmt $m $l2s --pen=2p,red >> $b
gmt $m $l2s --histtype=counts --kind=freq --kind=logcount+weights >> $b
gmt $m $l2s --kind=logfreq --kind=log10count --kind=log10freq >> $b

diff $a $b --strip-trailing-cr > fail
