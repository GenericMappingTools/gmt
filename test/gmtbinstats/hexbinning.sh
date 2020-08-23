#!/usr/bin/env bash
# Test hexagon tile binning on counting values in gmtbinstats
#
# Hardwire some values to draw the bottom plot so we avoid lots of shell math
# The commented stuff shows how the script and data were developed
#e=5
#n=3
#dy=1
#ny=$(gmt math -Q $n SUB $dy DIV =)
#r=$(gmt math -Q $dy 0.5 MUL 30 COSD DIV =)
#dx=$(gmt math -Q 3 $r MUL =)
#nx=$(gmt math -Q $e $dx DIV RINT 1 ADD =)
#e2=$(gmt math -Q 0 $nx 1 SUB $dx MUL ADD =)
#r=$(gmt math -Q $dy 30 COSD DIV =)
#echo "Revised: 0/$e2/0/$n -I$dx/$dy for a pseudo $nx by $ny grid with hexagon radius $r"
#dx2=$(gmt math -Q $dx 2 DIV =)
#dy2=$(gmt math -Q $dy 2 DIV =)
#wb=$(gmt math -Q 0 $dx2 ADD =)
#eb=$(gmt math -Q $e2 $dx2 SUB =)
#sb=$(gmt math -Q 0 $dy2 ADD =)
#nb=$(gmt math -Q $n $dy2 SUB =)
#rh=$(gmt math -Q 4 30 COSD DIV =)
#gmt math -T1/100/1 0 $e2 RAND = x
#gmt math -T1/100/1 0 $n RAND = y
#gmt math -T1/100/1 50 25 NRAND = z
#gmt convert -A x y z -o1,3,5 > hex_data.txt

# Create grids to find nodes of hexagons
gmt grdmath -R0/5.19615242271/0/3 -I1.73205080757/1 0 = b.grd
gmt grdmath -R0.866025403785/4.33012701893/0.5/2.5 -I1.73205080757/1 0 = r.grd
gmt grdmath -R0/5.19615242271/0/3 -I0.866025403785/0.5 0 = t.grd
gmt begin hexbinning ps
	gmt grd2xyz b.grd | gmt plot -Sh3.46410161514c -Glightblue@50 -W1p -N -R0/5.19615242271/0/3 -Jx3c -X3.2c
	gmt grd2xyz b.grd | gmt plot -Sh3.46410161514c -Glightblue -W1p
	gmt grd2xyz r.grd | gmt plot -Sh3.46410161514c -Glightred@50 -W1p -N
	gmt grd2xyz r.grd | gmt plot -Sh3.46410161514c -Glightred -W1p
	gmt grdmath -R0/5.19615242271/0/3 -I0.866025403785/0.5 0 = t.grd
	gmt grd2xyz t.grd | gmt plot -Sc0.2c -Gwhite -Wfaint -N
	gmt grd2xyz r.grd b.grd | gmt plot -Sc0.2c -Gblack -N
	gmt basemap -Bxafg0.866025403785 -Byafg0.5
	gmt plot hex_data.txt -Ss0.2c -Gyellow -Wfaint
	gmt binstats hex_data.txt -R0/5/0/3 -I1 -Th -Cn > bin.txt
	gmt makecpt -Cjet -T1/10/1 -A50
	gmt plot -Baf -Y12c bin.txt -C -Sh3.46410161514c -W1p -B+t"Hexagonal Binning"
	gmt text bin.txt -F+f14p,Helvetica-Bold -Gwhite -W0.25p -N
gmt end show
