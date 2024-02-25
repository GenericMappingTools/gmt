#!/usr/bin/env bash
#
# Test xyz2grd longopts translation.

m=xyz2grd
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Ad -Af -Al
--l2stranstest -Am -An -Ar
--l2stranstest -As -AS -Au
--l2stranstest -Az
--l2stranstest -D+c/some.cpt+dsillydata+n-99
--l2stranstest -D"+c-+o1+rSmart Grid"
--l2stranstest -D"+s2+tBest Grid Ever+vmyvar"
--l2stranstest -D+xDr.X+yDr.Y+zDr.Z
--l2stranstest -G/some/file.grd=nf+d2+n-99
--l2stranstest -G/other/file.grd=nf+o6+s1.5
--l2stranstest -I5+e/10 -I2 -I1/2+n
--l2stranstest -S -S
--l2stranstest -ZTLax
--l2stranstest -ZBRAyw
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
gmt $m $l2s --duplicate=difference --duplicate=first --duplicate=low >> $b
gmt $m $l2s --duplicate=mean --duplicate=number --duplicate=rms >> $b
gmt $m $l2s --duplicate=last --duplicate=stddev --duplicate=upper >> $b
gmt $m $l2s --multiple_nodes=sum >> $b
gmt $m $l2s --netcdf+cpt:/some.cpt+dname:sillydata+invalid:-99 >> $b
gmt $m $l2s --netCDF+cpt:-+offset:1+remark:'Smart Grid' >> $b
gmt $m $l2s --ncheader+scale:2+title:"Best Grid Ever"+varname:myvar >> $b
gmt $m $l2s --netcdf+xname:Dr.X+yname:Dr.Y+zname:Dr.Z >> $b
gmt $m $l2s --outgrid=/some/file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=/other/file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --increment=5+exact/10 --spacing=2 --inc=1/2+number >> $b
gmt $m $l2s --swap --byteswap >> $b
gmt $m $l2s --onecol=top,left,ascii,noxmax >> $b
gmt $m $l2s --one_col=bottom,right,ascii_float,noymax,byteswap >> $b
gmt $m $l2s --convention=top,right,int8,swap --flags=bottom,left,char  >> $b
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
