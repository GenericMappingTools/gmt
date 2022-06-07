#!/usr/bin/env bash
#
# Test multi-line titles with LaTeX subtitles using @^ short marker
ps=subtitle_latex.ps

gmt psbasemap -R-30/30/-20/20 -JX5i/2.5i -Baf -B+t"My extremely long title and awkward title@^must go over two longish lines@^or perhaps even three"+s"<math>\nabla^4w = 2 \pi\psi</math>" -P -K -Xc > $ps
gmt psbasemap -R -J -Baf -B+t"Bouguer Anomaly: @[2 \pi\rho G h@["+s"<math>\nabla^4w = 2 \pi\psi</math>" -O -Y5.5i >> $ps
