#!/bin/bash
#	$Id$
# Testing gmt logo options

ps=logos.ps

gmt psxy -R0/8.5/0/11 -Jx1i -X0 -Y0 -P -K -W4p << EOF > $ps
> vertical line
4.25	0.2
4.25	10.8
> top horizontal line
0.2	8.25
8.3	8.25
> middle horizontal line
0.2	5.5
8.3	5.5
> bottom horizontal line
0.2	2.75
8.3	2.75
EOF
# Logo on yellow background with outline and with gray shadow
gmt logo -W3.5i -O -K -F+p+glightyellow+s -Y0.5i -X0.375i >> $ps
# Logo on yellow background with outline and with darkred shadow
gmt logo -W3.5i -O -K -F+p+glightyellow+sdarkred -X4.25i >> $ps
# Logo on yellow background with outline and larger south clearance
gmt logo -W3.5i -O -K -F+p+glightyellow+c4p/4p/20p/4p -Y2.75i -X-4.25i >> $ps
# Logo on no background with double outline and unequal clearances
gmt logo -W3.5i -O -K -F+p+c12p/6p/20p/4p+i -X4.25i >> $ps
# Logo on yellow background with outline
gmt logo -W3.5i -O -K -F+p+glightyellow -Y2.75i -X-4.25i >> $ps
# Logo on yellow background without outline
gmt logo -W3.5i -O -K -F+glightyellow -X4.25i >> $ps
# Logo by itself
gmt logo -W3.5i -O -K -Y2.75i -X-4.25i >> $ps
# Logo with outline
gmt logo -W3.5i -O -K -F -X4.25i >> $ps
gmt psxy -R -J -O -T >> $ps
