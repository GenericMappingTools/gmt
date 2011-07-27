#!/bin/bash
#		$Id$
#
#	Testing GMT mapproject on examples in Snyder.
#
#
#	usage:  snyder.sh [-v]
#
#	-v will report values for each project. 
#	scipt will report trouble if we do not match Snyder (+- 360 degrees for longitudes)
#	Slop is 0.11 m for projected values

blabber () {
	gf=(`echo "${sf[*]}" | mapproject $* -F -C`)
	gi=(`echo "${gf[*]}" | mapproject $* -F -C -I --FORMAT_FLOAT_OUT=%lg`)
	if [ $blabber -gt 0 ] ; then
		echo "-------------------"
		echo $p
		echo "-------------------"
		echo "Snyder  x=${sf[0]}, y=${sf[1]} X=${si[0]} Y=${si[1]}"
		echo "GMT     x=${gi[0]}, y=${gi[1]} X=${gf[0]} Y=${gf[1]}"
	fi
	echo "${sf[*]} ${gi[*]} ${si[*]} ${gf[*]} $p" | awk -f test.awk
}

. ../functions.sh

blabber=0
if [ $# -gt 0 ] ; then
	blabber=1
else
	header "Compare mapproject forward/inverse with Snyder"
fi

gmtset PROJ_SCALE_FACTOR 1

# Make ellipsoids (spheres) of radius 1 and 3:
unit=1.0
three=3.0
# Make special Clarke-1866 ellipsoid using Snyders value for e^2=0.00676866
# which gives 1/f = 294.97861076:
snyder=6378206.4,294.97861076
# International 1924 ellipsoid
int=International-1924

#----------------------------------------------------------------------------
# Mercator, Spherical

sf=(-75 35)
si=(1.8325957 0.6528366)
p="Mercator(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R0/360/-50/50 -Jm180/0/1:1 > fail
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Mercator, Ellipsoidal

sf=(-75 35)
si=(11688673.7 4139145.6)
p="Mercator(Ellipsoidal)"
blabber --PROJ_ELLIPSOID=$snyder --FORMAT_FLOAT_OUT=%9.1lf -R0/360/-50/50 -Jm180/0/1:1 >> fail
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Transverse Mercator, Spherical

sf=(-73.5 40.5)
si=(0.0199077 0.7070276)
p="TM(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R-77/-72/35/45 -Jt-75/0/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Oblique Mercator, Spherical
# Commented out since GMT and Snyder use different origins...
#
#sf=(120 -30)
#si=(-2.4201335 -0.0474026)
#p="Oblique-Mercator(Spherical)"
#blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R0/360/-80/80 -Job0/45/-90/0/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Cylindrical Equal-Area, Spherical

sf=(80 35)
si=(2.3428242 0.6623090)
p="Cylindrical-Equal-Area(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R-255/105/-80/80 -Jy-75/30/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Cylindrical Equal-Area, Ellipsoidal

sf=(-78 5)
si=(-332699.8 554248.5)
p="Cylindrical-Equal-Area(Ellipsoidal)"
blabber --PROJ_ELLIPSOID=$snyder --PROJ_SCALE_FACTOR=0.9996 --FORMAT_FLOAT_OUT=%9.1lf -R-255/105/-80/80 -Jy-75/5/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Miller, Spherical

sf=(-75 50)
si=(-1.3089969 0.9536371)
p="Miller(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R-180/180/-80/80 -Jj0/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Cassini, Spherical

sf=(-90 25)
si=(-0.2367759 0.7988243)
p="Cassini(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R-77/-73/-23/-17 -Jc-75/-20/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Cassini, Ellipsoidal

sf=(-73 43)
si=(163071.1 335127.6)
p="Cassini(Ellipsoidal)"
blabber --PROJ_ELLIPSOID=$snyder --FORMAT_FLOAT_OUT=%9.1lf -R-76/-72/38/42 -Jc-75/40/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Albers, Spherical

sf=(-75 35)
si=(0.2952720 0.2416774)
p="Albers(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R-100/-90/30/40 -Jb-96/23/29.5/45.5/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Albers, Ellipsoidal

sf=(-75 35)
si=(1885472.7 1535925.0)
p="Albers(Ellipsoidal)"
blabber --PROJ_ELLIPSOID=$snyder --FORMAT_FLOAT_OUT=%9.1lf -R-100/-90/30/40 -Jb-96/23/29.5/45.5/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Lambert Conic, Spherical

sf=(-75 35)
si=(0.2966785 0.2462112)
p="Lambert-Conic(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R-100/-90/30/40 -Jl-96/23/33/45/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Lambert Conic, Ellipsoidal

sf=(-75 35)
si=(1894410.9 1564649.5)
p="Lambert-Conic(Ellipsoidal)"
blabber --PROJ_ELLIPSOID=$snyder --FORMAT_FLOAT_OUT=%9.1lf -R-100/-90/30/40 -Jl-96/23/33/45/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Equidistant Conic, Spherical

sf=(-75 35)
si=(0.2952057 0.2424021)
p="Equidistant-Conic(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R-100/-90/30/40 -Jd-96/23/29.5/45.5/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Stereographic, Spherical

sf=(-75 30)
si=(0.3807224 -0.1263802)
p="Stereographic(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R0/360/-90/90 -Js-100/40/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Stereographic, Ellipsoidal

sf=(-90 30)
si=(971630.8 -1063049.3)
p="Stereographic-General(Ellipsoidal)"
blabber --PROJ_ELLIPSOID=$snyder --PROJ_SCALE_FACTOR=0.9999 --FORMAT_FLOAT_OUT=%9.1lf -R0/360/-90/90 -Js-100/40/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Stereographic, Ellipsoidal

sf=(150 -75)
si=(-1573645.4 -572760.1)
p="Stereographic-Polar(Ellipsoidal)"
blabber --PROJ_ELLIPSOID=$int --PROJ_SCALE_FACTOR=0.994 --FORMAT_FLOAT_OUT=%9.1lf -R0/360/-90/0 -Js-100/-90/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Gnomonic, Spherical

sf=(-110 30)
si=(-0.1542826 -0.1694739)
p="Gnomonic(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R0/360/-90/90 -Jf-100/40/60/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Lambert Azim, Spherical

sf=(100 -20)
si=(-4.2339303 4.0257775)
p="Lambert-Azimuthal(Spherical)"
blabber --PROJ_ELLIPSOID=$three --FORMAT_FLOAT_OUT=%9.7lf -R0/360/-90/90 -Ja-100/40/180/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Lambert Azim, Ellipsoidal Polar

sf=(5 80)
si=(1077459.7 288704.5)
p="Lambert-Azimuthal-Polar(Ellipsoidal)"
blabber --PROJ_ELLIPSOID=$int --FORMAT_FLOAT_OUT=%9.1lf -R0/360/0/90 -Ja-100/90/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Lambert Azim, Ellipsoidal

sf=(-110 30)
si=(-965932.1 -1056814.9)
p="Lambert-Azimuthal-General(Ellipsoidal)"
blabber --PROJ_ELLIPSOID=$snyder --FORMAT_FLOAT_OUT=%9.1lf -R0/360/-90/90 -Ja-100/40/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Azim Equidistant, Spherical

sf=(100 -20)
si=(-5.8311398 5.5444634)
p="Azimuthal-Equidistant(Spherical)"
blabber --PROJ_ELLIPSOID=$three --FORMAT_FLOAT_OUT=%9.7lf -R0/360/-90/90 -Je-100/40/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Van der Grinten, Spherical

sf=(-160 -50)
si=(-1.1954154 -0.9960733)
p="Van-der-Grinten(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R95/455/-90/90 -Jv-85/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Sinusoidal, Spherical

sf=(-75 -50)
si=(0.1682814 -0.8726646)
p="Sinusoidal(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R90/450/-90/90 -Ji-90/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Mollweide, Spherical

sf=(-75 -50)
si=(0.1788845 -0.9208758)
p="Mollweide(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R90/450/-90/90 -Jw-90/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Eckert IV, Spherical

sf=(-75 -50)
si=(0.1875270 -0.9519210)
p="Eckert-IV(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R90/450/-90/90 -Jkf-90/1:1 >> fail

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Eckert VI, Spherical

sf=(-75 -50)
si=(0.1693623 -0.9570223)
p="Eckert-VI(Spherical)"
blabber --PROJ_ELLIPSOID=$unit --FORMAT_FLOAT_OUT=%9.7lf -R90/450/-90/90 -Jks-90/1:1 >> fail

#----------------------------------------------------------------------------

if [ $blabber -eq 0 ]; then
	passfail snyder
else
	cat fail
fi
