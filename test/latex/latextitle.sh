#!/usr/bin/env bash
#
# Test LaTeX equations in titles
ps=latextitle.ps
gmt set FONT_TITLE 24p,Times-Roman,black
gmt psbasemap -R-30/30/-20/20 -JX6i/2.5i -Byaf -B+t'Evaluating the Solution to @[\nabla^4 \Psi = \frac{\delta w}{\delta t} = f(x(t))@[' -P -K -Xc -Y1.25i > $ps
gmt psbasemap -R -J -Baf -B+t"Evaluating Some Nasty Equation" -O -Y5.25i >> $ps
