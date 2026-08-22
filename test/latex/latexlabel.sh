#!/usr/bin/env bash
#
# Test LaTeX equations in labels
ps=latexlabel.ps
gmt psbasemap -R-30/30/-20/20 -JX6i/2.5i -Byaf+l'@[\Delta\sigma_{xx}^2@[ (MPa)' -Bxaf+l'Evaluating @[\nabla^4 \Psi = \frac{\delta w}{\delta t} = f(x(t))@[' -B+t"The Main Results of my Research" -P -K -Xc -Y1.25i > $ps
gmt psbasemap -R -J -Byaf+l'@[\Delta\sigma_{xx}^2@[ (MPa)' -Bxaf+l'Evaluating @[\nabla^4 \Psi = \frac{\delta w}{\delta t} = f(x(t))@['  -B+t"My Extremely Long Title with Subtitle:"+s"Here are Main Conclusions of My Research" -O -Y5.25i >> $ps
