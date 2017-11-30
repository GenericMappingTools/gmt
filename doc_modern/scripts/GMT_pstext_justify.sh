#!/bin/bash
#	$Id$
#
B=0.2
M=0.38
T=0.56
L=0.10
C=1.04
R=1.98
gmt pstext -R0/3/0/1.5 -Jx1i -P -K -N -C0 -Wthin,- -F+f36p,Helvetica-Bold+jLB << EOF > GMT_pstext_justify.ps
0.1	0.2	My Text
EOF
gmt psxy -R -J -O -K -N << EOF >> GMT_pstext_justify.ps
>
0.05	$B
2.04	$B
>
0.05	$M
2.04	$M
>
0.05	$T
2.04	$T
>
$L	0
$L	0.65
>
$C	0
$C	0.65
>
$R	0
$R	0.65
EOF
gmt psxy -R -J -O -K -N -Wthinner << EOF >> GMT_pstext_justify.ps
>
0.7	-0.1
$L	$M
>
1.3	-0.1
$R	$T
EOF
gmt pstext -R -J -O -K -N -F+f8p+j << EOF >> GMT_pstext_justify.ps
$L	0.69	CB	L (Left)
$C	0.69	CB	C (Center)
$R	0.69	CB	R (Right)
2.07	$T	LM	T (Top)
2.07	$M	LM	M (Middle)
2.07	$B	LM	B (Bottom)
0.6	-0.05	LM	LM
1.37	-0.05	RM	TR
EOF
gmt psxy -R -J -O -Sc0.05 << EOF >> GMT_pstext_justify.ps
$L	$B
$L	$M
$L	$T
$C	$B
$C	$M
$C	$T
$R	$B
$R	$M
$R	$T
EOF
