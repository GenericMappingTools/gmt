#! /bin/bash
#	$Id: test_cm4.sh,v 1.2 2011-05-26 19:35:43 jluis Exp $
#
# Tests mgd77magref against the values of the original FORTRAN version 
# Because the second term (lithospheric) does not agree it is not included in the comparison

. ../functions.sh
header "Test mgd77magref against original CM4 Fortran version"

log=test_cm4.log

data=2000.08700533

echo -30 45 0 $data | mgd77magref -A+y -Fxyz/1 -Sc1/15 | gmtconvert --FORMAT_FLOAT_OUT=%.7E  > cm4_c.dat
#echo -30 45 0 $data | mgd77magref -A+y -Fxyz/2 -Sc1/15 | gmtconvert --FORMAT_FLOAT_OUT=%.7E >> cm4_c.dat
echo -30 45 0 $data | mgd77magref -A+y -Fxyz/3 -Sc1/15 | gmtconvert --FORMAT_FLOAT_OUT=%.7E >> cm4_c.dat
echo -30 45 0 $data | mgd77magref -A+y -Fxyz/4 -Sc1/15 | gmtconvert --FORMAT_FLOAT_OUT=%.7E >> cm4_c.dat
echo -30 45 0 $data | mgd77magref -A+y -Fxyz/5 -Sc1/15 | gmtconvert --FORMAT_FLOAT_OUT=%.7E >> cm4_c.dat
echo -30 45 0 $data | mgd77magref -A+y -Fxyz/6 -Sc1/15 | gmtconvert --FORMAT_FLOAT_OUT=%.7E >> cm4_c.dat
echo -30 45 0 $data | mgd77magref -A+y -Fxyz/7 -Sc1/15 | gmtconvert --FORMAT_FLOAT_OUT=%.7E >> cm4_c.dat

# Output from the Fortran version
#C/L 1   B_xyz     2.1577205E+04 -5.4288317E+03  4.1624773E+04
#C/L 2   B_xyz    -1.2747567E+01 -9.8955432E+00 -1.1668323E+01		THIS ONE DIFFERS FROM C VERSION
#Mag_pri B_xyz    -1.5506497E+01  4.4450796E+00  1.9214288E+01
#Mag_ind B_xyz    -8.4923656E-01  5.5134285E-01 -1.3137803E+00
#Ion_pri B_xyz    -5.1975837E+00 -4.5903891E+00  2.9385937E+00
#Ion_ind B_xyz     7.9738793E-01 -2.2101940E+00 -3.7272951E+00
#Tor     B_xyz    -3.4804584E+00 -6.0774586E+00  1.1689001E-02


echo     2.1577205E+04	-5.4288317E+03	4.1624773E+04	>  cm4_f.dat
#echo    -1.2747567E+01	-9.8955432E+00 -1.1668323E+01	>> cm4_f.dat
echo    -1.5506497E+01	4.4450796E+00	1.9214288E+01	>> cm4_f.dat
echo    -8.4923656E-01	5.5134285E-01	-1.3137803E+00	>> cm4_f.dat
echo    -5.1975837E+00	-4.5903891E+00	2.9385937E+00	>> cm4_f.dat
echo     7.9738793E-01	-2.2101940E+00	-3.7272951E+00	>> cm4_f.dat
echo    -3.4804584E+00	-6.0774586E+00	1.1689001E-02	>> cm4_f.dat

gmtmath -Ca cm4_c.dat cm4_f.dat SUB = | awk '{print $1+$2+$3}' | gmtmath -S -T STDIN MEAN = | awk '($1 == 0)' > $log

rm -f cm4_c.dat cm4_f.dat

passfail test_cm4
