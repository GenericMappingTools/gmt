#!/usr/bin/env bash
#
# Tests the IGRF harmonic degree band limiting, i.e. gmt mgd77magref -F.../0+l<low>/<high>
#
# The X,Y,Z components are linear in the spherical harmonic coefficients, so any split of
# the degree range must add back up to the unrestricted field.  That is checked here for
# two different splits, plus a few pinned values and the option error checking.

log=igrf_band.log

cat << EOF > pts.txt
-20	35	0	2020.0
110	-45	10	2020.5
0	90	0	2005.0
30	-10	400	1980.0
EOF

# -Vq silences the warning issued when the requested degree 13 is truncated to 10 for the
# pre-1995 models, which is itself tested further down.  We ask for all the digits we can
# get since the residuals below are computed from these printed values.
fmt=--FORMAT_FLOAT_OUT=%.15e
gmt mgd77magref pts.txt -A+y -Fxyz/0       -Vq $fmt > full.dat
gmt mgd77magref pts.txt -A+y -Fxyz/0+l1/5  -Vq $fmt > lo.dat
gmt mgd77magref pts.txt -A+y -Fxyz/0+l6/13 -Vq $fmt > hi.dat
gmt mgd77magref pts.txt -A+y -Fxyz/0+l1/1  -Vq $fmt > dip.dat
gmt mgd77magref pts.txt -A+y -Fxyz/0+l2/13 -Vq $fmt > nodip.dat

echo "Degrees 1-5 plus 6-13 must equal all degrees:" > $log
paste lo.dat hi.dat full.dat | $AWK '{for (j = 1; j <= 3; j++) {d = $j + $(j+3) - $(j+6); if (d < 0) d = -d; if (d > max) max = d}}
	END {printf "%s\n", (max < 1e-6) ? "residual < 1e-6 nT" : "FAIL: residual is " max " nT"}' >> $log
echo "Degree 1 plus degrees 2-13 must equal all degrees:" >> $log
paste dip.dat nodip.dat full.dat | $AWK '{for (j = 1; j <= 3; j++) {d = $j + $(j+3) - $(j+6); if (d < 0) d = -d; if (d > max) max = d}}
	END {printf "%s\n", (max < 1e-6) ? "residual < 1e-6 nT" : "FAIL: residual is " max " nT"}' >> $log

# Asking for all the degrees the model has must reproduce the unrestricted field exactly
echo "Explicitly asking for degrees 1-13 must be the same as not restricting at all:" >> $log
gmt mgd77magref pts.txt -A+y -Fxyz/0+l1/13 -Vq $fmt | diff - full.dat --strip-trailing-cr > /dev/null && echo "identical" >> $log

# Pinned values (F, X, Y, Z) at lon = -20, lat = 35, sea level, 2020.0
echo "Total field and components for the full field, degree 1 only, and degrees 6-13:" >> $log
for band in "" "+l1/1" "+l6/13"; do
	echo -20 35 0 2020.0 | gmt mgd77magref -A+y -Ftxyz/0$band --FORMAT_FLOAT_OUT=%.4f >> $log
done

# The modifier only applies to the IGRF, so it must be refused for any CM4 combination
echo "Number of errors when +l is used with CM4 (-Ft/1) or with the mixed mode (-Ft/934):" >> $log
gmt mgd77magref pts.txt -A+y -Ft/1+l1/5   2>&1 | grep -c "only available for the IGRF model" >> $log
gmt mgd77magref pts.txt -A+y -Ft/934+l1/5 2>&1 | grep -c "only available for the IGRF model" >> $log
echo "Number of errors for a malformed band (+l5), a reversed band (+l5/2) and a zero degree (+l0/3):" >> $log
gmt mgd77magref pts.txt -A+y -Ft/0+l5   2>&1 | grep -c "usage is +l<low>/<high>" >> $log
gmt mgd77magref pts.txt -A+y -Ft/0+l5/2 2>&1 | grep -c "requires 1 <= <low> <= <high>" >> $log
gmt mgd77magref pts.txt -A+y -Ft/0+l0/3 2>&1 | grep -c "requires 1 <= <low> <= <high>" >> $log

# The IGRF only has degrees up to 10 before 1995, so a higher upper degree must be truncated
echo "Number of truncation warnings for a 1980 date with +l1/13:" >> $log
echo 30 -10 400 1980.0 | gmt mgd77magref -A+y -Ft/0+l1/13 2>&1 | grep -c "only has harmonic degrees up to 10" >> $log
echo "Truncating degree 13 to 10 for 1980 must give the complete 1980 field:" >> $log
echo 30 -10 400 1980.0 | gmt mgd77magref -A+y -Ftxyz/0+l1/13 -Vq --FORMAT_FLOAT_OUT=%.4f > trunc.dat
echo 30 -10 400 1980.0 | gmt mgd77magref -A+y -Ftxyz/0 --FORMAT_FLOAT_OUT=%.4f | diff - trunc.dat --strip-trailing-cr > /dev/null && echo "identical" >> $log

diff $log "${src:-.}"/igrf_band.txt --strip-trailing-cr | tee fail > /dev/null
