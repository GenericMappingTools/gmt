#!/usr/bin/env bash
# Test that proj info is well propagated into nc and gtiff files and that grdimage
# produces well referenced geotiffs

ps=grdimage_img_tif.ps

gmt grdmath -R0/20/30/50 -I0.25 X Y MUL = lixo.grd
gmt grdproject lixo.grd -J+proj=utm+zone=32 -Glixo_utm.grd
gmt grdproject lixo.grd -J+proj=utm+zone=32 -Glixo_utm.tiff=gd:GTiff

gmt grdimage lixo_utm.grd  -JX8c -I+d -Ba -BWSen -P -K > $ps
echo 10 40 | gmt mapproject -J+proj=utm+zone=32 | gmt psxy -JX8c -Rlixo_utm.grd -Sc1c -Gwhite -O -K >> $ps
gmt grdimage lixo_utm.tiff -JX8c -I+d -Ba -BwSen -X8.5c -O -K >> $ps
echo 10 40 | gmt mapproject -J+proj=utm+zone=32 | gmt psxy -JX8c -Rlixo_utm.tiff -Sc1c -Gwhite -O -K >> $ps

gmt grdimage lixo_utm.grd -I+d -Agrdimg.tiff

gmt grdimage grdimg.tiff -JX8c -X-8.5c -Y8.5c -Ba -BWseN -O -K >> $ps
echo 10 40 | gmt mapproject -J+proj=utm+zone=32 | gmt psxy -JX8c -Rgrdimg.tiff -Sc1c -Gwhite -O >> $ps
