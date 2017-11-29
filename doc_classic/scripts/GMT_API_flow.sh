#!/bin/bash
ps=GMT_API_flow.ps
gmt pstext -R-2/3/-1/1 -Jx1i -P -F+f14p,Helvetica-Bold -Gwhite -W0.5p -N -K -C50% -X2i << EOF > $ps
-2	0	EXTERNAL INTERFACE
0	0	gmt
1.27	0	parser
3.1	0	GMT C/C++ API
EOF
gmt psxy -R -J -O -K -W1p+v0.15i+gblack+h0.5 << EOF >> $ps
>
-0.8	0
-0.27	0
>
0.27	0
0.88	0
>
1.66	0
2.30	0
EOF
gmt pstext -R -J -O -K -F+f48p,Helvetica,gray -N << EOF >> $ps
0.60	0.06	[
1.97	0.06	]
EOF
gmt psxy -R -J -O -T >> $ps
