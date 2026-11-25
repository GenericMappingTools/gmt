#!/usr/bin/env bash
#
# Test gravfft -T...+m (gravity of the flexed moho) against the two-step
# equivalent: -Q to get the moho surface, then -D to get its gravity.
# Both describe the same surface, only the reference plane differs, so
# away from the edges the two anomalies must differ by a constant.
# See https://github.com/GenericMappingTools/gmt/issues/7255

echo "0	0	60	4000" | gmt grdseamount -R-256/256/-256/256 -I2 -r -Cg -Gsmt.nc -Z-5000
# One step: gravity of the flexed moho
gmt gravfft smt.nc+uk -T10000/2800/3300/1030+m -Z15000 -E4 -Nf -Gmoho_g1.nc
# Two steps: moho surface, then its gravity using rho_m - rho_l
gmt gravfft smt.nc+uk -T10000/2800/3300/1030 -Z15000 -Q -Nf -Gmoho_z.nc
gmt gravfft moho_z.nc+uk -D500 -E4 -Nf -Gmoho_g2.nc
# Compare the interior of the two results; std must be ~0
gmt grdmath moho_g1.nc moho_g2.nc SUB = diff.nc
gmt grdcut diff.nc -R-128/128/-128/128 -Gdiff_in.nc
if [ -f diff_in.nc ]; then
	gmt grdinfo -C -L2 diff_in.nc | awk '{if ($13 > 0.01) printf "Std of difference too large: %g mGal\n", $13}' > fail
else	# Some command failed (e.g., gravfft -T+m crashed) so there is nothing to compare
	echo "No difference grid was produced" > fail
fi
