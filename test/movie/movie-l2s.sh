#!/usr/bin/env bash
#
# Test movie longopts translation.

m=movie
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C4320p
--l2stranstest -Nstarwars
--l2stranstest -T25 -T0/100/5+n
--l2stranstest -T/my/timefile+p4 -T250+s100
--l2stranstest -T"10+w :;-"
--l2stranstest -T10+W
--l2stranstest -A/some/audio/file.mp3+e
--l2stranstest -D32
--l2stranstest -Esomescript+d8s+f3s+ggreen
--l2stranstest -Esomescript+fi6+fo4s
--l2stranstest -Fgif+l7 -Fmp4+osomeopts
--l2stranstest -Fwebm+s5 -Fpng+t+v
--l2stranstest -G25/125/77+p2p
--l2stranstest -H3
--l2stranstest -I/include/file.txt
--l2stranstest -K+fi4+gwhite+pi -K+fo8s+po
--l2stranstest -Le+s0.5+c1p/2p+ggreen+p2p
--l2stranstest -LsSomeString+fhelvetica+gred+hgray
--l2stranstest -Lf+jTL -Lp+o1/1
--l2stranstest -Lc5+ggreen+r -Lt7+t%s
--l2stranstest -M20,tif+r12 -Mf
--l2stranstest -Pa+fhelvetica -Pb+ac7
--l2stranstest -Pc+gred+Ggreen -Pd+p2p+P1p
--l2stranstest -Pf+jTR+o2/1+s2 -Pe+t%s+w3
--l2stranstest -Q -Qs
--l2stranstest -Sbbgscript.sh -Sffgscript.sh
--l2stranstest -W/some/directory
--l2stranstest -Z -Zs
EOF

# module-specific longopts
gmt $m $l2s --canvassize=4320p >> $b
gmt $m $l2s --name=starwars >> $b
gmt $m $l2s --frames=25 --frames=0/100/5+nframes >> $b
gmt $m $l2s --frames=/my/timefile+tagwidth:4 --frames=250+first:100 >> $b
gmt $m $l2s --frames=10+wordsepall:' :;-' >> $b
gmt $m $l2s --frames=10+wordseptab >> $b
gmt $m $l2s --audio=/some/audio/file.mp3+exact >> $b
gmt $m $l2s --displayrate=32 >> $b
gmt $m $l2s --title=somescript+duration:8s+fadetime:3s+fadecolor:green >> $b
gmt $m $l2s --title=somescript+fadetime:i6+fadetime:o4s >> $b
gmt $m $l2s --videoformat=gif+loop:7 --videoformat=mp4+encode:someopts >> $b
gmt $m $l2s --videoformat=webm+stride:5 --videoformat=png+transparent+view >> $b
gmt $m $l2s --fill=25/125/77+pen:2p >> $b
gmt $m $l2s --tempscale=3 >> $b
gmt $m $l2s --include=/include/file.txt >> $b
gmt $m $l2s --fade+length:i4+color:white+preserve:i --fade+length:o8s+preserve:o >> $b
gmt $m $l2s --label=e+scale:0.5+clearance:1p/2p+fill:green+pen:2p >> $b
gmt $m $l2s --label=sSomeString+font:helvetica+fill:red+shade:gray >> $b
gmt $m $l2s --label=f+refpoint:TL --label=p+offset:1/1 >> $b
gmt $m $l2s --label=c5+fill:green+rounded --label=t7+format:'%s' >> $b
gmt $m $l2s --master=20,tif+altdpu:12 --master=first >> $b
#gmt $m $l2s --master=middle,png+view --master=last,+view >> $b
gmt $m $l2s --progress=pie+font:helvetica --progress=wheel+annotate:c7 >> $b
gmt $m $l2s --progress=arrow+mfill:red+sfill:green --progress=line+mpen:2p+spen:1p >> $b
gmt $m $l2s --progress=axis+justify:TR+offset:2/1+scale:2 --progress=gauge+format:'%s'+width:3 >> $b
gmt $m $l2s --debug --debug=scripts >> $b
gmt $m $l2s --static=bg:bgscript.sh --static=fg:fgscript.sh >> $b
gmt $m $l2s --workdir=/some/directory >> $b
gmt $m $l2s --delete --delete=scripts >> $b

diff $a $b --strip-trailing-cr > fail
