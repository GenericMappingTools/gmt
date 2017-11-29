#!/bin/bash
ps=GMT_API_use.ps
gmt psxy -R-4.5/4.5/-2/2 -Jx0.8i -P -K -W2p+ve0.2i+gblack+h0.5 -Xc << EOF > $ps
>
-2	0.75
-0.65	0.4
>
-1.8	0
-0.65	0
>
-1.9	-0.75
-0.65	-0.4
EOF
gmt psxy -R -J -O -K -W2p+ve0.2i+gblack+h0.5  << EOF >> $ps
0.65	0.4
1.9	0.75
>
0.5	0
1.15	0
>
0.65	-0.4
1.775	-0.75
EOF
gmt psxy -R -J -O -L -K -W1p << EOF >> $ps
> -Gpink
-0.65	-0.5
0.65	-0.5
0.65	0.5
-0.65	0.5
EOF
gmt pstext -R -J -O -K -F+f16p,Helvetica-Bold+jCM << EOF >> $ps
0 0.2 GMT
0 -0.2 MODULE
EOF
gmt pstext -R -J -O -K -F+f12p+jCM -Gwhite -W0.25p -C50% << EOF >> $ps
-2.8 0.75 FILES OR STDIN
+2.8 0.75 FILES OR STDIN
EOF
gmt pstext -R -J -O -K -F+f12p+jCM -Glightblue -W0.25p -C50% << EOF >> $ps
-2.8 0.0 STREAMS, FILE DESCRIPTORS
+2.8 0.0 STREAMS, FILE DESCRIPTORS
EOF
gmt pstext -R -J -O -K -F+f12p+jCM -Glightorange -W0.25p -C50% << EOF >> $ps
-2.8 -0.75 GMT CONTAINERS
+2.8 -0.75 GMT CONTAINERS
EOF
gmt psxy -R -J -O -T >> $ps
