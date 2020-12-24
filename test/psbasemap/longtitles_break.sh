#!/usr/bin/env bash
#
# Test multi-line titles and subtitles using <break> marker
ps=longtitles_break.ps

gmt psbasemap -R-30/30/-20/20 -JX5i/2.5i -Baf -B+t"My extremely long title and awkward title<break>must go over two longish lines<break>or perhaps even three" -P -K -Xc > $ps
gmt psbasemap -R -J -Baf -B+t"My extremely long title and awkward title:"+s"Subtitles must go over two longish lines<break>or perhaps even three" -O -Y4.75i >> $ps
