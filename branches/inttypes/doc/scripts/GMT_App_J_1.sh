#!/bin/bash
#	$Id$
#
# Script to draw the impulse responses and transfer functions
# for GMT cookbook Appendix_J.
#
# W H F Smith, 18 December 1998.
#
# 1-dimensional case, "filter1d", Fourier transform.
#
# Impulse responses (relative amplitude, x in units of
#   length per filter width).
#
# Let distance units be expressed relative to filter width,
# i.e. x = s/w, where s is the user's distance unit and w
# is the user's filter width in the argument to filter1d.
# Then the impulse responses are non-zero only for fabs(x) < 0.5.
# The impulse responses for fabs(x) < 0.5 are proportional to:
#    boxcar:    h(x) = 1.0;
#    cosine:    h(x) = 0.5 * (1.0 + cos(2 * pi * x) );
#    gaussian:  h(x) = exp (-18 * x * x);
# The factor 18 comes from the fact that we use sigma = 1/6
# in these units and a definition of the gaussian with the
# factor 1/2 as in the normal probability density function.
#
# Transfer functions (f = frequency in cycles per filter width):
# H(f) = integral from -0.5 to 0.5 of h(x) * cos(2 * pi * f * x).
#    boxcar:    H(f) = (sin (pi f) ) / (pi * f);
#    cosine:    H(f) = ((sin (pi f) ) / (pi * f)) / ( 1.0 - f*f);
#    gaussian:  H(f) ~ exp (-(1/18) * (pi * f) * (pi * f) );
# The gaussian H(f) is approximate because the convolution is
# carried only over fabs(x) < 0.5 and not fabs(x) -> infinity.
# Of course, all these H(f) are approximate because the discrete
# sampling of the data is not accounted for in these formulae.
#
#
# 2-dimensional case, "grdfilter", Hankel transform.
#
# Impulse response:
#   Let r be measured in units relative to the filter width.
#   The filter width defines a diameter, so the impulse 
#   response is non-zero only for r < 0.5, as for x above.
#   So the graph of the impulse response h(r) for 0 < r < 0.5
#   is identical to the graph for h(x) for 0 < x < 0.5 above.
#
# Transfer functions:
#   These involve the Hankel transform of h(r):
# H(q) = 2 * pi * Integral from 0 to 0.5 of h(r) * J0(2piqr) r dr
# as in Bracewell, p. 248, where J0 is the zero order Bessel function,
# and q is in cycles per filter width.
#    boxcar:    H(q) = J1 (2 * pi * q) / (pi * q);  J1 = 1st order Bessel fn.
#    cosine:    H(q) = must be evaluated numerically (?**).
#    gaussian:  H(q) ~ exp (-(1/18) * (pi * q) * (pi * q) );
#
# After many hours of tedium and consulting the treatises like Watson,
# I gave up on obtaining an answer for the Hankel transform of the
# cosine filter.  I tried substituting an infinite series of J(4k)(z)
# for 1 + cos(z), and I tried substituting an integral form of J0(z).
# Nothing worked out.
#
# I tried to compute the Hankel transform numerically on the HP, and
# found that the -lm library routines j0(x) and j1(x) give wrong answers.
# I used an old Sun to compute "r_tr_fns.d" for plotting here.
#
# NOTE that the expressions in the comments above are not the actual
# impulse responses because they are normalized to have a maximum
# value of 1.  Direct Fourier or Hankel transform of these values
# gives a transfer function with H(0) not equal 1, generally.  I
# have normalized the transfer functions (correctly) so that H(0)=1.
# Thus the graphs of H are correct, but the graphs of h(x) are only
# relative.  One reason for doing it this was is that then the
# graphs of h(x) can be interpreted as also = the graphs of h(r).
#
#---------------------------------------------------
echo "-0.5	0" > tt.tmp
gmtmath -T-0.5/0.5/0.01 1 = >> tt.tmp
echo "0.5	0" >> tt.tmp

gmtset FONT_ANNOT_PRIMARY 10p,Times-Roman FONT_TITLE 14p,Times-Roman FONT_LABEL 12p,Times-Roman
psxy tt.tmp -R-0.6/0.6/-0.1/1.1 -JX4i/2i -P -Ba0.5f0.1:"Distance (units of filter width)":/a0.2f0.1g1:"Relative amplitude":WeSn -K -Wthick > GMT_App_J_1.ps
gmtmath -T-0.5/0.5/0.01 T PI 2 MUL MUL COS 1 ADD 0.5 MUL = | psxy -R -J -O -K -Wthick,- >> GMT_App_J_1.ps
gmtmath -T-0.5/0.5/0.01 T T MUL 18 MUL NEG EXP = | psxy -R -J -O -K -Wthick,. >> GMT_App_J_1.ps
pstext -R -J -O -F+f9p,Times-Roman+j << END >> GMT_App_J_1.ps
-0.2	0.3	LM	Solid Line:
-0.2	0.2	LM	Dotted Line:
-0.2	0.1	LM	Dashed Line:
0.2	0.3	RM	Boxcar
0.2	0.2	RM	Gaussian
0.2	0.1	RM	Cosine
END
