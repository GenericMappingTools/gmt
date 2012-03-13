#! /bin/bash
#	$Id$
#
# Tests mgd77magref against the values of the original FORTRAN version 
# Because the second term (lithospheric) does not agree it is not included in the comparison

. ./functions.sh
header "Test mgd77magref vs original CM4 Fortran version"

data=2000.08700533

rm -f test_cm4.dat
for val in 1 2 3 4 5 6 7; do
	echo -30 45 0 $data | mgd77magref -A+y -Fxyz/$val -Sc1/15 | gmtconvert --FORMAT_FLOAT_OUT=%20.10f >> test_cm4.dat
done

# Output from the Fortran version
#C/L 1   B_xyz     2.1577205E+04 -5.4288317E+03  4.1624773E+04
#C/L 2   B_xyz    -1.2747567E+01 -9.8955432E+00 -1.1668323E+01		THIS ONE DIFFERS FROM C VERSION
#Mag_pri B_xyz    -1.5506497E+01  4.4450796E+00  1.9214288E+01
#Mag_ind B_xyz    -8.4923656E-01  5.5134285E-01 -1.3137803E+00
#Ion_pri B_xyz    -5.1975837E+00 -4.5903891E+00  2.9385937E+00
#Ion_ind B_xyz     7.9738793E-01 -2.2101940E+00 -3.7272951E+00
#Tor     B_xyz    -3.4804584E+00 -6.0774586E+00  1.1689001E-02

diff test_cm4.dat "$src"/test_cm4.dat --strip-trailing-cr > fail

passfail test_cm4
