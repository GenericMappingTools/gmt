#!/usr/bin/env bash
#
# Test psconvert longopts translation.

m=psconvert
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A+r -A+u
--l2stranstest -Cspooky
--l2stranstest -D/this/dir -D/that/dir
--l2stranstest -E1000
--l2stranstest -Fshortname -Flongname -Fprefix
--l2stranstest -G/a/bc -G/d/e/f
--l2stranstest -H3
--l2stranstest -I+m1/2/3/4+sm400+S10
--l2stranstest -L/my/psfilelist -L/your/psfilelist
--l2stranstest -Mbbg.ps -Mbbg.ps
--l2stranstest -Mffg.ps -Mffg.ps
--l2stranstest -N+f50+ggreen+kblue
--l2stranstest -N+gorange+p1p,red
--l2stranstest -Qg1 -Qp2
--l2stranstest -Qt4
--l2stranstest -S
--l2stranstest -Tb -Te+m -TE+q75
--l2stranstest -Tf -TF -Tj -Tg
--l2stranstest -TG -Tm -Tt+m
--l2stranstest -W+aG+c+f0.5/0.8+g
--l2stranstest -W+c+k+l1/2+nThick
--l2stranstest -W+o/dir+tMyDoc+uhttp://worldly.com/x.html
--l2stranstest -Z -Z
EOF

# module-specific longopts
gmt $m $l2s --adjust+round --crop+no_timestamp >> $b
gmt $m $l2s --gs_option=spooky >> $b
gmt $m $l2s --outdir=/this/dir --out_dir=/that/dir >> $b
gmt $m $l2s --dpi=1000 >> $b
gmt $m $l2s --outfile=shortname --out_name=longname --prefix=prefix >> $b
gmt $m $l2s --gs_path=/a/bc --ghost_path=/d/e/f >> $b
gmt $m $l2s --scale=3 >> $b
gmt $m $l2s --resize+margins:1/2/3/4+size:m400+scale:10 >> $b
gmt $m $l2s --listfile=/my/psfilelist --list_file=/your/psfilelist >> $b
gmt $m $l2s --pslayer=bg:bg.ps --pslayer=background:bg.ps >> $b
gmt $m $l2s --pslayer=fg:fg.ps --pslayer=foreground:fg.ps >> $b
gmt $m $l2s --bgcolor+fade:50+bg:green+fadecolor:blue >> $b
gmt $m $l2s --bgcolor+background:orange+pen:1p,red >> $b
gmt $m $l2s --anti_aliasing=graphics:1 --anti_aliasing=geopdf:2 >> $b
gmt $m $l2s --anti_aliasing=text:4 >> $b
gmt $m $l2s --gs_command >> $b
gmt $m $l2s --format=bmp --fmt=eps+mono --format=pageszeps+quality:75 >> $b
gmt $m $l2s --format=pdf --format=multipdf --format=jpeg --format=png >> $b
gmt $m $l2s --format=transpng --format=ppm --format=tiff+monochrome >> $b
gmt $m $l2s --world_file+altitude:G+nocrop+fade:0.5/0.8+gdal >> $b
gmt $m $l2s --world_file+no_crop+kml+lod:1/2+layer:Thick >> $b
gmt $m $l2s --esri+folder:/dir+doc:MyDoc+url:http://worldly.com/x.html >> $b
gmt $m $l2s --remove_infile --del_input_ps >> $b

diff $a $b --strip-trailing-cr > fail
