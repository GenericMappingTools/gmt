#!/usr/bin/env bash
#
# Test grd2xyz longopts translation.

m=grd2xyz
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C/some/file.cpt -C/some/file.cpt
--l2stranstest -Ff -Ff -Fi
--l2stranstest -Lc -Lc
--l2stranstest -Lr -Lx -Ly
--l2stranstest -Ta5 -Ta -Tb0.8
--l2stranstest -Wa+uin -W2
--l2stranstest -ZTLax
--l2stranstest -ZBRay
--l2stranstest -ZTRcw -ZBLc
--l2stranstest -ZTLu -ZTLu
--l2stranstest -ZTLh -ZTLh
--l2stranstest -ZTLH -ZTLH
--l2stranstest -ZTLi -ZTLi
--l2stranstest -ZTLI -ZTLI
--l2stranstest -ZTLl -ZTLl
--l2stranstest -ZTLL -ZTLL
--l2stranstest -ZTLf -ZTLf
--l2stranstest -ZTLd -ZTLd
EOF

# module-specific longopts
gmt $m $l2s --cpt=/some/file.cpt --cmap=/some/file.cpt >> $b
gmt $m $l2s --rowcol=one --row_col=fortran --rowcol=indexz >> $b
gmt $m $l2s --single=col --hvline=column >> $b
gmt $m $l2s --array=row --single=x --single=y >> $b
gmt $m $l2s --stl=ascii:5 --stl=ASCII --stl=binary:0.8 >> $b
gmt $m $l2s --weight=area+unit:in --weight=2 >> $b
gmt $m $l2s --onecol=top,left,ascii,noxmax >> $b
gmt $m $l2s --one_col=bottom,right,ASCII,noymax >> $b
gmt $m $l2s --ordering=top,right,int8,swap --onecol=bottom,left,char  >> $b
gmt $m $l2s --onecol=top,left,uint8 --onecol=top,left,uchar >> $b
gmt $m $l2s --onecol=top,left,int16 --onecol=top,left,short >> $b
gmt $m $l2s --onecol=top,left,uint16 --onecol=top,left,ushort >> $b
gmt $m $l2s --onecol=top,left,int32 --onecol=top,left,int >> $b
gmt $m $l2s --onecol=top,left,uint32 --onecol=top,left,uint >> $b
gmt $m $l2s --onecol=top,left,int64 --onecol=top,left,long >> $b
gmt $m $l2s --onecol=top,left,uint64 --onecol=top,left,ulong >> $b
gmt $m $l2s --onecol=top,left,float32 --onecol=top,left,float >> $b
gmt $m $l2s --onecol=top,left,float64 --onecol=top,left,double >> $b

diff $a $b --strip-trailing-cr > fail
