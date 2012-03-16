#!/bin/bash
#	$Id$
#
pstext -R0/5/0/3 -Jx1i -P -K -F+f+j << EOF > GMT_-J.ps
2.5	2.8	16p,Helvetica-Bold	BC	GMT PROJECTIONS
2	2.25	12p,Helvetica-Bold	BC	GEOGRAPHIC PROJECTIONS
0	1.75	11p,Helvetica		BL	CYLINDRICAL
1.1	1.75	11p,Helvetica		BL	CONICAL
2	1.75	11p,Helvetica		BL	AZIMUTHAL
3	1.75	11p,Helvetica		BL	THEMATIC
4	1.75	11p,Helvetica		BL	OTHER
EOF
pstext -R -J -O -K -F+f8p+jBL << EOF >> GMT_-J.ps
# Cylindrical
0	1.35	Basic [E]
0	1.2	Cassini
0	1.05	Equidistant
0	0.9	Mercator [C]
0	0.75	Miller
0	0.6	Oblique Mercator [C]
0	0.45	Stereographic
0	0.3 	Transverse Mercator [C]
0	0.15	UTM [C]
# Conical
1.1	1.35	Albers [E]
1.1	1.2	Equidistant
1.1	1.05	Lambert [C]
1.1	0.9	Polyconic
# Azimuthal
2	1.35	Equidistant
2	1.2	Gnomonic
2	1.05	Orthographic
2	0.9	Perspective
2	0.75	Lambert [E]
2	0.6	Stereographic [C]
# Thematic
3	1.35	Eckert IV + VI [E]
3	1.2	Hammer [E]
3	1.05	Mollweide [E]
3	0.9	Robinson
3	0.75	Sinusoidal [E]
3	0.6	Winkel Tripel
3	0.45	Van der Grinten
# Other
4	1.35	Linear
4	1.2	Logarithmic
4	1.05	Exponential
4	0.9	Time
4	0.75	Polar
#
0.05	2.75	C = Conformal
0.05	2.6	E = Equal Area
EOF

psxy -R -J -O -Wthinner << EOF >> GMT_-J.ps
>
2.3	2.75
2	2.4
>
2.7	2.75
4.2	1.9
>
1.7	2.2
0.2	1.9
>
1.9	2.2
1.3	1.9
>
2.1	2.2
2.2	1.9
>
2.3	2.2
3.2	1.9
>
0.2	1.7
0.2	1.5
>
1.3	1.7
1.3	1.5
>
2.2	1.7
2.2	1.5
>
3.2	1.7
3.2	1.5
>
4.2	1.7
4.2	1.5
>
0	2.55
0.85	2.55
0.85	2.87
0	2.87
0	2.55
EOF
