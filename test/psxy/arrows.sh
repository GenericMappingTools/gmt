#!/usr/bin/env bash
ps=arrows.ps
xr=$(gmt math -Q 1 15 COSD MUL 1.5 ADD =)
xri=$(gmt math -Q 1 15 COSD MUL 1.5 SUB =)
xl=$(gmt math -Q $xr NEG =)
xli=$(gmt math -Q $xri NEG =)
gmt psbasemap -R-3/3/0/9 -Jx1i -P -Baf -K -Xc > $ps
gmt psxy -R -J -O -K -W0.5p << EOF >> $ps
> -W0.5p
-1.5	0
-1.5	9
>
1.5	0
1.5	9
> -W0.5p,-
$xl	0
$xl	9
>
$xr	0
$xr	9
>
$xli	0
$xli	9
>
$xri	0
$xri	9
EOF
echo 0 8.8 0 3i | gmt psxy -R -J -O -K -Sv1i+jc+h0.5 -W6p --PS_MITER_LIMIT=0 >> $ps
echo 0 8.2 0 3i | gmt psxy -R -J -O -K -Sv1i+ea+jc+h0.5 -W6p -Gred --PS_MITER_LIMIT=0 >> $ps
echo 0 7.6 0 3i | gmt psxy -R -J -O -K -Sv1i+ba+jc+h0.5 -W6p -Gred --PS_MITER_LIMIT=0 >> $ps
echo 0 7.0 0 3i | gmt psxy -R -J -O -K -Sv1i+ea+bi+jc+h0.5 -W6p -Gred --PS_MITER_LIMIT=0 >> $ps
echo 0 6.4 0 3i | gmt psxy -R -J -O -K -Sv1i+ba+ei+jc+h0.5 -W6p -Gred --PS_MITER_LIMIT=0 >> $ps
echo 0 5.8 0 3i | gmt psxy -R -J -O -K -Sv1i+eA+jc+h0.5 -W6p -Gred --PS_MITER_LIMIT=0 >> $ps
echo 0 5.2 0 3i | gmt psxy -R -J -O -K -Sv1i+bA+jc+h0.5 -W6p -Gred --PS_MITER_LIMIT=0 >> $ps
echo 0 4.6 0 3i | gmt psxy -R -J -O -K -Sv1i+eA+bI+jc+h0.5 -W6p -Gred --PS_MITER_LIMIT=0 >> $ps
echo 0 4.0 0 3i | gmt psxy -R -J -O -K -Sv1i+bA+eI+jc+h0.5 -W6p -Gred --PS_MITER_LIMIT=0 >> $ps
echo 0 3.4 0 3i | gmt psxy -R -J -O -K -Sv1i+eA+jc+h0.5 -W6p -Gred --PS_LINE_CAP=round >> $ps
echo 0 2.8 0 3i | gmt psxy -R -J -O -K -Sv1i+bA+jc+h0.5 -W6p -Gred --PS_LINE_CAP=round >> $ps
echo 0 2.2 0 3i | gmt psxy -R -J -O -K -Sv1i+eA+bI+jc+h0.5 -W6p -Gred --PS_LINE_CAP=round >> $ps
echo 0 1.6 0 3i | gmt psxy -R -J -O -K -Sv1i+bA+eI+jc+h0.5 -W6p -Gred --PS_LINE_CAP=round >> $ps
echo 0 1.0 0 3i | gmt psxy -R -J -O -K -Sv1i+bI+eI+jc+h0.5 -W6p -Gred --PS_LINE_CAP=round >> $ps
echo 0 0.4 0 3i | gmt psxy -R -J -O -Sv1i+bA+eA+jc+h0.5 -W6p -Gred --PS_LINE_CAP=round >> $ps
