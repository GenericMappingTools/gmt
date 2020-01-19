#!/usr/bin/env bash
gmt begin GMT_API_use
	gmt plot -R-4.5/4.5/-2/2 -Jx0.8i -W2p+ve0.2i+gblack+h0.5 -Xc << EOF
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
	gmt plot -W2p+ve0.2i+gblack+h0.5  << EOF
0.65	0.4
1.9	0.75
>
0.5	0
1.15	0
>
0.65	-0.4
1.775	-0.75
EOF
	gmt plot -L -W1p << EOF
> -Gpink
-0.65	-0.5
0.65	-0.5
0.65	0.5
-0.65	0.5
EOF
	gmt text -F+f16p,Helvetica-Bold+jCM << EOF
0 0.2 GMT
0 -0.2 MODULE
EOF
	gmt text -F+f12p+jCM -Gwhite -W0.25p -C50% << EOF
-2.8 0.75 FILES OR STDIN
+2.8 0.75 FILES OR STDIN
EOF
	gmt text -F+f12p+jCM -Glightblue -W0.25p -C50% << EOF
-2.8 0.0 STREAMS, FILE DESCRIPTORS
+2.8 0.0 STREAMS, FILE DESCRIPTORS
EOF
	gmt text -F+f12p+jCM -Glightorange -W0.25p -C50% << EOF
-2.8 -0.75 GMT CONTAINERS
+2.8 -0.75 GMT CONTAINERS
EOF
gmt end show
