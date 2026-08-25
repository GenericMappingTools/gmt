#!/usr/bin/env bash
# Test pstext paragraph mode
ps=book.ps
gmt pstext -R0/6/0/9 -Jx1i -P -B0 -F+f+jCM -K << EOF > $ps
3	8	46p	A Tale of Two Cities
3	7	32p	Dickens, Charles
3	6.4	24p	1812-1973
EOF
# First Paragraph
gmt pstext -R -J -F+f16p,Times-Roman,red+jTC -O -M << EOF >> $ps
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

	There were a king with a large jaw and a queen with a plain face,
on the throne of England; there were a king with a large jaw and
a queen with a fair face, on the throne of France.  In both
countries it was clearer than crystal to the lords of the State
preserves of loaves and fishes, that things in general were
settled for ever.
EOF
