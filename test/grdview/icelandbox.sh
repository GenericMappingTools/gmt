#!/usr/bin/env bash
#
# Script to plot 3D model of Iceland, provided by Peter Schmidt
#

ps=icelandbox.ps

### Plot options
popt=-p130/30

### Colormap
cpt=resid.cpt
cat << EOF > $cpt
#	cpt file created by: makecpt -T-9/9/2 but then manually doctored
#COLOR_MODEL = +HSV
#
-11	0	0	0	-9	0	0	0	L
-9	255	1	0.6	-7	255	1	0.6	L
-7	240	1	1	-5	240	1	1	L
-5	216.667	1	1	-3	216.667	1	1	L
-3	183.333	1	1	-1	183.333	1	1	L
-1	0	0	0.85	1	0	0	0.85	L
1	116.667	1	1	3	116.667	1	1	L
3	83.3333	1	1	5	83.3333	1	1	L
5	50	1	1	7	50	1	1	L
7	19.6667	1	1	9	19.6667	1	1	L
9	1.0	1	1	11	1.0	1	1	B
B	0	0	0
F	0	0	1
N	0	0	0.501961
EOF

### Pivotal points
a="-24.4130658 63.0225642"
b="-17.49029 63.1705533"
c="-17.4678686 64.5788931"
d="-13.5105157 64.5134051"
e="-13.121354 66.7035842"
f="-25.3398981 66.6019215"

### Set up walls between 0 and 200 km depth
gmt psxyz $popt -R-24.4/63.0/-13.1/66.7/0/200r -JZ-5c -JT-17.927/64.463/15c -Bxa2f1 -Bya1f0.5 -Bz50 -BESZ -Wthinnest -K << EOF > $ps
> -G220
$a 200
$a 0
$b 0
$b 200
>
$c 200
$c 0
$d 0
$d 200
> -G180
$b 200
$b 0
$c 0
$c 200
>
$d 200
$d 0
$e 0
$e 200
EOF

gmt psxyz -p -R -J -JZ -Wthinnest -O -K << EOF >> $ps
$a 100
$b 100
$c 100
$d 100
$e 100
EOF

### Set clipping perimeter
gmt psclip $popt/0 -R -JZ -J -O -K << EOF >> $ps
$a
$b
$c
$d
$e
$f
EOF

### Start plotting gmt surface
gmt grdimage @D3-25TV24-resid.nc -E100 -nl -p -R -J -JZ -C$cpt -O -K >> $ps

gmt grdcontour --PS_COMMENTS=1 @D3-25TV24-resid.nc -p -R -J -JZ -Wthinner -An -C$cpt -W -K -O >> $ps

gmt pscoast -p -R -J -JZ -Dh -A100 -Wthinnest -S135/190/240 -O -K >> $ps
gmt psclip -C -O >> $ps

