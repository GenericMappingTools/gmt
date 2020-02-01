#!/usr/bin/env bash
# Show placements of columns on a variable surface
ps=3dbars.ps
gmt grdmath -R0/10E/0/10N -I1 X Y MUL = t.nc
gmt makecpt -Cjet -T0/100 > t.cpt
# Create a few x,y,z
cat << EOF > t.txt
2	2	70
5	3	20
1	8	100
8	8	30
3	9	50
8	2	45
EOF
# Use z as base and  add bar height to base via +B
gmt grdtrack -Gt.nc t.txt > junk.txt
gmt grdview t.nc -Ct.cpt -Qi100 -JM4i -JZ1.5i -R0/10/0/10/0/150 -p155/35 -P -Baf -Bzaf -BWSNEZ+b -K -X1.5i > $ps
gmt psxyz -R -J -JZ -O -K junk.txt -So0.2i+B -Gred -W0.25p -p >> $ps
# Show same columns on a flat surface
gmt grdimage t.nc -Ct.cpt -J -JZ -R -p -O -Baf -Bzaf -BWSNEZ+b -K -Y4.5i >> $ps
gmt psxyz -R -J -JZ -O t.txt -So0.2i+b0 -Gred -W0.25p -p >> $ps
