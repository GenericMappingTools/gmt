#!/usr/bin/env bash

# Testing gmt math for LSQFIT and SVDFIT solutions to Ax = b
ps=lsfit.ps
# (bottom)
# Dummy data is y(x) = a + bx + c*H(x-x0) + N(0,0.1), with
# a = 1, b = 0.1, c = 2, and x0 = 0.5, for x -3:3, with
# N being normally distributed Gaussian noise, std = 0.1
a=1
b=0.1
c=2
x0=0.5
# The next line produced stepdata.txt, now in subversion
# gmt math -T-3/3/0.1 T $b MUL $a ADD $x0 STEPT $c MUL ADD 0 0.1 NRAND ADD = stepdata.txt
gmt math -A@stepdata.txt+r -N4/1 -C0 1 ADD -C1 T ADD -C2 0.5 STEPT ADD -Ca LSQFIT = solution.txt
# For this example we use the coefficients to evaluate the function the hard way
q=($(cat solution.txt))
gmt math -T-3/3/0.1 T ${q[1]} MUL ${q[0]} ADD 0.5 STEPT ${q[2]} MUL ADD = stepfit_lsq.txt
gmt math -A@stepdata.txt+r -N4/1 -C0 1 ADD -C1 T ADD -C2 0.5 STEPT ADD -Ca SVDFIT = solution.txt
q=($(cat solution.txt))
gmt math -T-3/3/0.1 T ${q[1]} MUL ${q[0]} ADD 0.5 STEPT ${q[2]} MUL ADD = stepfit_svd.txt
gmt psxy -R-3/3/0/4 -JX6.5i/4i -P -Baf -BWSne @stepdata.txt -Sc0.15c -Gblue -K -Xc > $ps
gmt psxy -R -J -O -K stepfit_lsq.txt -W2p >> $ps
gmt psxy -R -J -O -K stepfit_svd.txt -Sc2p -Gred >> $ps
gmt pstext -R -J -O -K -F+f12p+jRB+cRB+t"Fit y(x) = a + b*x + c*H(x-$x0) + @~e@~(x)" -Dj0.2i >> $ps
gmt pslegend -R -J -O -K -DjTL+w1.75i+jTL+o0.1i/0.1i -F+p << EOF >> $ps
S 0.1i - 0.15i - 2p 0.3i LSQFIT solution
S 0.1i c 2p  red - 0.3i SVDFIT solution
S 0.1i c 0.15c  blue - 0.3i Input data
EOF
# (top)
# Dummy data is y(x) = a + b*cos(2*pi*6*x/X-c) + N(0,0.1), with
# a = 1, b = 2, c = 2, and X = 2, for x 0:2, with
# N being normally distributed Gaussian noise, std = 0.1
a=1
b=2
c=2
X=2
# The next line produced sinusoiddata.txt, now in subversion
# gmt math -T0/2/0.01 T $X DIV 2 MUL PI MUL 6 MUL $c SUB COS $b MUL $a ADD 0 0.2 NRAND ADD  = sinusoiddata.txt
# Here, we instead use the +s modifier to return the solution evaluated at the data points.
gmt math -A@sinusoiddata.txt+e+r -N4/1 -C0 1 ADD -C1,2 T ADD 2 DIV 2 MUL PI MUL 6 MUL -C1 COS -C2 SIN -Ca LSQFIT = sinusoidfit_lsq.txt
gmt math -A@sinusoiddata.txt+e+r -N4/1 -C0 1 ADD -C1,2 T ADD 2 DIV 2 MUL PI MUL 6 MUL -C1 COS -C2 SIN -Ca SVDFIT = sinusoidfit_svd.txt
gmt psxy -R0/2/-3/4 -J -O -Baf @sinusoiddata.txt -Sc0.15c -Gblue -K -Y5i >> $ps
gmt psxy -R -J -O -K sinusoidfit_lsq.txt -W2p -i0,2 >> $ps
gmt psxy -R -J -O -K sinusoidfit_svd.txt -Sc2p -Gred -i0,2 >> $ps
gmt pstext -R -J -O -K -F+f12p+jRB+cRB+t"Fit y(x) = a + b*cos(6@~p@~x - c) + @~e@~(x) = a + b@-1@-*cos(6@~p@~x) + b@-2@-*sin(6@~p@~x) + @~e@~(x)" -Dj0.2i >> $ps
gmt psxy -R -J -O -T >> $ps
