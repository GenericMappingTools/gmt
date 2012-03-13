#!/bin/bash
# $Id$
#
# This is original Figure 4 script from
# Wessel, P. (2010), Tools for analyzing intersecting tracks: the x2sys package,
# Computers & Geosciences, 36, 348–354.
# Here used as a test for the x2sys suite.

. ./functions.sh
header "Reproduce Wessel (2010) Comp. & Geosci., Figure 4"

OLDX=$X2SYS_HOME
export X2SYS_HOME=.

rm -rf TEST
ln -fs "$src"/bad "$src"/data .
(cd bad; ls *.xyg) > bad.lis
(cd data; ls *.xyg) > data.lis

x2sys_init TEST -D${GMT_SOURCE_DIR}/share/x2sys/geoz -Exyg -F -G -R180/185/0/5
echo "bad" >> TEST/TEST_paths.txt
x2sys_cross -TTEST =data.lis -Qe -Ia > COE_clean.txt
x2sys_cross -TTEST =bad.lis -Qe -Ia > COE_orig.txt
x2sys_list -TTEST COE_orig.txt -Cz -Fndc > COE_use.txt
x2sys_solve -TTEST COE_use.txt -Cz -Ed > corrections.txt
R=181/185/0/3
makecpt -Crainbow -T-80/80/10 -Z > faa.cpt

# Grid the corrected data
x2sys_datalist -TTEST -Lcorrections.txt =bad.lis -Flon,lat,z | blockmean -R$R -I1m | surface -R$R -I1m -Gss_gridded_fix1.nc -T0.25
grdgradient ss_gridded_fix1.nc -Ne0.75 -A65 -fg -Gss_gridded_fix1_int.nc
grdimage ss_gridded_fix1.nc -Iss_gridded_fix1_int.nc -Ei -JM5.5i -P -K -Cfaa.cpt -B1Wsne -X1.75i -Y5.75i --MAP_FRAME_WIDTH=3p --FORMAT_GEO_MAP=dddF > $ps

# Obtain adjustments and grid the corrected and adjusted data
x2sys_report -TTEST -Cz COE_orig.txt -Lcorrections.txt -A > /dev/null
x2sys_datalist -TTEST -A -Lcorrections.txt =bad.lis -Flon,lat,z | blockmean -R$R -I1m | surface -R$R -I1m -Gss_gridded_fix2.nc -T0.25
grdgradient ss_gridded_fix2.nc -Ne0.75 -A65 -fg -Gss_gridded_fix2_int.nc
grdimage ss_gridded_fix2.nc -Iss_gridded_fix2_int.nc -Ei -JM5.5i -O -K -Cfaa.cpt -B1WSne -Y-4.5i --MAP_FRAME_WIDTH=3p --FORMAT_GEO_MAP=dddF >> $ps
psxy -R -J -O -T >> $ps

if [ ! "X$OLDX" = "X" ]; then	# Reset prior setting
	export X2SYS_HOME=$OLDX
fi
pscmp
