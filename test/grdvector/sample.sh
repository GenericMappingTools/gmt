#!/usr/bin/env bash
# Test sub-sampling of grids for vectors
ps=sample.ps
gmt pscoast -R60/105/-20/40 -JM95.0/35/16c -Gbisque -K -Bafg8 -P -Xc > $ps
gmt grdvector nuvel1.vx.1.5.nc nuvel1.vy.1.5.nc -R -J -I2 -Si100k -Q0.2i+e+n200k -Wthicker,lightgray -Glightgray -O -K -t50 >> $ps
gmt grdvector nuvel1.vx.1.5.nc nuvel1.vy.1.5.nc -R -J -I8 -Si100k -Q0.2i+e+n200k -Wthicker,black -Gred -O >> $ps
