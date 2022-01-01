#!/usr/bin/env bash
# Test pstext paragraph mode
ps=shade_text.ps
gmt pstext -R0/6/0/9 -Jx1i -P -B0 -F+f+jCM -K -Glightgreen -S -Xc << EOF > $ps
3	8	46p	A TALE OF
EOF
gmt pstext -R -J -F+f+jCM -O -K -Gwhite -W0.5p -S -C+tO << EOF >> $ps
3	7	46p	TWO CITIES!
EOF
# First Paragraph
gmt pstext -R -J -F+f16p,Times-Roman,red+jTC -O -M -Glightblue -W2p -S8p/-8p/darkblue -C8p+tc << EOF >> $ps
> 3 5 18p 5i j
	@_It was the best of times, it was the worst of times@_,
it was the age of wisdom, it was the age of foolishness,
it was the epoch of belief, it was the epoch of incredulity,
it was the season of Light, it was the season of Darkness,
it was the spring of hope, it was the winter of despair,
we had everything before us, we had nothing before us,
we were all going direct to Heaven, we were all going direct
the other way--in short, the period was so far like the present
period, that some of its noisiest authorities insisted on its
being received, for good or for evil, in the superlative degree
of comparison only.
EOF
