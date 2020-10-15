#!/usr/bin/env bash
# Test command-line args to -Se -SE -Sj -SJ in psxyz
ps=cmdargs.ps
echo 0 0 0 > t.txt
gmt set MAP_FRAME_TYPE plain
gmt psxyz -R-3/3/-3/3 -JX2i -Se30/1i/0.5i -Glightblue -W1p -Baf -BWSrt t.txt -P -K -X2i -Y0.6i > $ps
gmt psxyz -R -J -Se0.7i -Glightblue -W1p -Baf -BWSrt t.txt -O -K -X3i >> $ps
gmt psxyz -R -J -Sj30/1i/0.5i -Glightblue -W1p -Baf -BWSrt t.txt -O -K -X-3i -Y2.6i >> $ps
gmt psxyz -R -J -Sj0.7i -Glightblue -W1p -Baf -BWSrt t.txt -O -K -X3i >> $ps
gmt psxyz -R-10/10/-10/10 -JM2i -SE30/1000k/500k -Glightblue -W1p -Baf -BWSrt t.txt -O -K -X-3i -Y2.6i >> $ps
gmt psxyz -R -J -SE1000k -Glightblue -W1p -Baf -BWSrt t.txt -O -K -X3i >> $ps
gmt psxyz -R -J -SJ30/1000k/500k -Glightblue -W1p -Baf -BWSrt t.txt -O -K -X-3i -Y2.6i >> $ps
gmt psxyz -R -J -SJ1000k -Glightblue -W1p -Baf -BWSrt t.txt -O -X3i >> $ps
