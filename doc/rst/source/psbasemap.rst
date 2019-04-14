.. index:: ! psbasemap

*********
psbasemap
*********

.. only:: not man

    psbasemap - Plot base maps and frames

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**psbasemap** |-J|\ *parameters*
|SYN_OPT-Rz|
[ |SYN_OPT-B| ]
[ |-A|\ [*file*] ]
[ |-D|\ *insetbox* ]
[ |-F|\ *box* ]
[ |-K| ]
[ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-L|\ *scalebar* ]
[ |-O| ]
[ |-P| ]
[ |SYN_OPT-U| ]
[ |-T|\ *rose* ]
[ |-T|\ *mag_rose* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: basemap_common.rst_

.. include:: common_classic.rst_

.. _-D:

**-D**\ [*unit*]\ *xmin/xmax/ymin/ymax*\ [**r**][**+s**\ *file*][**+t**] \| **-D**\ [**g**\ \|\ **j**\ \|\ **J**\ \|\ **n**\ \|\ **x**]\ *refpoint*\ **+w**\ *width*\ [/*height*][**+j**\ *justify*][**+o**\ *dx*\ [/*dy*]][**+s**\ *file*][**+t**]
    Draw a simple map inset box on the map.  Requires **-F**.  Specify the box in one of three ways:
    (a) Give *west/east/south/north* of geographic rectangle bounded by parallels
    and meridians; append **r** if the coordinates instead are the lower left and
    upper right corners of the desired rectangle. (b) Give **u**\ *xmin/xmax/ymin/ymax*
    of bounding rectangle in projected coordinates (here, **u** is the coordinate unit).
    (c) Give the reference point on the map for the inset using one of four coordinate systems:
    (1) Use **-Dg** for map (user) coordinates, (2) use **-Dj** or **-DJ** for setting *refpoint* via
    a 2-char justification code that refers to the (invisible) map domain rectangle,
    (3) use **-Dn** for normalized (0-1) coordinates, or (4) use **-Dx** for plot coordinates
    (inches, cm, etc.).
    Append **+w**\ *width*\ [/*height*] of bounding rectangle or box in plot coordinates (inches, cm, etc.).
    By default, the anchor point on the scale is assumed to be the bottom left corner (BL), but this
    can be changed by appending **+j** followed by a 2-char justification code *justify* (see :doc:`pstext`).
    Note: If **-Dj** is used then *justify* defaults to the same as *refpoint*,
    if **-DJ** is used then *justify* defaults to the mirror opposite of *refpoint*.
    Add **+o** to offset the inset fig by *dx*/*dy* away from the *refpoint* point in
    the direction implied by *justify* (or the direction implied by **-Dj** or **-DJ**).
    If you need access to the placement of the lower left corner of the map inset and
    its dimensions in the current map unit, use **+s**\ *file* to write this information
    to *file*.  Alternatively, you may append **+t** to translate the plot origin to
    the lower left corner of the map inset.
    Specify inset box attributes via the **-F** option [outline only].

Examples
--------

The following section illustrates the use of the options by giving some
examples for the available map projections. Note how scales may be given
in several different ways depending on the projection. Also note the use
of upper case letters to specify map width instead of map scale.

Non-geographical Projections
----------------------------

Linear x-y plot
~~~~~~~~~~~~~~~

To make a linear x/y frame with all axes, but with only left and bottom
axes annotated, using xscale = yscale = 1.0, ticking every 1 unit and
annotating every 2, and using xlabel = "Distance" and ylabel = "No of samples", use

   ::

    gmt psbasemap -R0/9/0/5 -Jx1 -Bf1a2 -Bx+lDistance -By+l"No of samples" -BWeSn -P > linear.ps

Log-log plot
~~~~~~~~~~~~

To make a log-log frame with only the left and bottom axes, where the
x-axis is 25 cm and annotated every 1-2-5 and the y-axis is 15 cm and
annotated every power of 10 but has tick-marks every 0.1, run

   ::

    gmt psbasemap -R1/10000/1e20/1e25 -JX25cl/15cl -Bx2+lWavelength -Bya1pf3+lPower -BWS -P > loglog.ps

Power axes
~~~~~~~~~~

To design an axis system to be used for a depth-sqrt(age) plot with
depth positive down, ticked and annotated every 500m, and ages annotated
at 1 my, 4 my, 9 my etc, use

   ::

    gmt psbasemap -R0/100/0/5000 -Jx1p0.5/-0.001 -Bx1p+l"Crustal age" -By500+lDepth -P > power.ps

Polar (theta,r) plot
~~~~~~~~~~~~~~~~~~~~

For a base map for use with polar coordinates, where the radius from 0
to 1000 should correspond to 3 inch and with gridlines and ticks intervals
automatically determined, use

   ::

    gmt psbasemap -R0/360/0/1000 -JP6i -Bafg -P > polar.ps

Cylindrical Map Projections
---------------------------

Cassini
~~~~~~~

A 10-cm-wide basemap using the Cassini projection may be obtained by

   ::

    gmt psbasemap -R20/50/20/35 -JC35/28/10c -P -Bafg -B+tCassini > cassini.ps

Mercator [conformal]
~~~~~~~~~~~~~~~~~~~~

A Mercator map with scale 0.025 inch/degree along equator, and showing
the length of 5000 km along the equator (centered on 1/1 inch), may be
plotted as

   ::

    gmt psbasemap -R90/180/-50/50 -Jm0.025i -Bafg -B+tMercator -Lx1i/1i+c0+w5000k -P > mercator.ps

Miller
~~~~~~

A global Miller cylindrical map with scale 1:200,000,000 may be plotted as

   ::

    gmt psbasemap -Rg -Jj180/1:200000000 -Bafg -B+tMiller -P > miller.ps

Oblique Mercator [conformal]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To create a page-size global oblique Mercator basemap for a pole at
(90,30) with gridlines every 30 degrees, run

   ::

    gmt psbasemap -R0/360/-70/70 -Joc0/0/90/30/0.064cd -B30g30 -B+t"Oblique Mercator" -P > oblmerc.ps

Transverse Mercator [conformal]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A regular Transverse Mercator basemap for some region may look like

   ::

    gmt psbasemap -R69:30/71:45/-17/-15:15 -Jt70/1:1000000 -Bafg -B+t"Survey area" -P > transmerc.ps

Equidistant Cylindrical Projection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This projection only needs the central meridian and scale. A 25 cm wide
global basemap centered on the 130E meridian is made by

   ::

    gmt psbasemap -R-50/310/-90/90 -JQ130/25c -Bafg -B+t"Equidistant Cylindrical" -P > cyl_eqdist.ps

Universal Transverse Mercator [conformal]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To use this projection you must know the UTM zone number, which defines
the central meridian. A UTM basemap for Indo-China can be plotted as

   ::

    gmt psbasemap -R95/5/108/20r -Ju46/1:10000000 -Bafg -B+tUTM -P > utm.ps

Cylindrical Equal-Area
~~~~~~~~~~~~~~~~~~~~~~

First select which of the cylindrical equal-area projections you want by
deciding on the standard parallel. Here we will use 45 degrees which
gives the Gall projection. A 9 inch wide global basemap centered
on the Pacific is made by

   ::

    gmt psbasemap -Rg -JY180/45/9i -Bafg -B+tGall -P > gall.ps

Conic Map Projections
---------------------

Albers [equal-area]
~~~~~~~~~~~~~~~~~~~

A basemap for middle Europe may be created by

   ::

    gmt psbasemap -R0/90/25/55 -Jb45/20/32/45/0.25c -Bafg -B+t"Albers Equal-area" -P > albers.ps

Lambert [conformal]
~~~~~~~~~~~~~~~~~~~

Another basemap for middle Europe may be created by

   ::

    gmt psbasemap -R0/90/25/55 -Jl45/20/32/45/0.1i -Bafg -B+t"Lambert Conformal Conic" -P > lambertc.ps

Equidistant
~~~~~~~~~~~

Yet another basemap of width 6 inch for middle Europe may be created by

   ::

    gmt psbasemap -R0/90/25/55 -JD45/20/32/45/6i -Bafg -B+t"Equidistant conic" -P > econic.ps

Polyconic
~~~~~~~~~

A basemap for north America may be created by

   ::

    gmt psbasemap -R-180/-20/0/90 -JPoly/4i -Bafg -B+tPolyconic -P > polyconic.ps

Azimuthal Map Projections
-------------------------

Lambert [equal-area]
~~~~~~~~~~~~~~~~~~~~

A 15-cm-wide global view of the world from the vantage point -80/-30
will give the following basemap:

   ::

    gmt psbasemap -Rg -JA-80/-30/15c -Bafg -B+t"Lambert Azimuthal" -P > lamberta.ps

Follow the instructions for stereographic projection if you want to
impose rectangular boundaries on the azimuthal equal-area map but
substitute **-Ja** for **-Js**.

Equidistant
~~~~~~~~~~~

A 15-cm-wide global map in which distances from the center (here 125/10)
to any point is true can be obtained by:

   ::

    gmt psbasemap -Rg -JE125/10/15c -Bafg -B+tEquidistant -P > equi.ps

Gnomonic
~~~~~~~~

A view of the world from the vantage point -100/40 out to a horizon of
60 degrees from the center can be made using the Gnomonic projection:

   ::

    gmt psbasemap -Rg -JF-100/40/60/6i -Bafg -B+tGnomonic -P > gnomonic.ps

Orthographic
~~~~~~~~~~~~

A global perspective (from infinite distance) view of the world from the
vantage point 125/10 will give the following 6-inch-wide basemap:

   ::

    gmt psbasemap -Rg -JG125/10/6i -Bafg -B+tOrthographic -P > ortho.ps

General Perspective
~~~~~~~~~~~~~~~~~~~

The **-JG** option can be used in a more generalized form, specifying
altitude above the surface, width and height of the view point, and
twist and tilt. A view from 160 km above -74/41.5 with a tilt of 55 and
azimuth of 210 degrees, and limiting the viewpoint to 30 degrees width
and height will product a 6-inch-wide basemap:

   ::

    gmt psbasemap -Rg -JG-74/41.5/160/210/55/30/30/6i -Bafg -B+t"General Perspective" -P > genper.ps

Stereographic [conformal]
~~~~~~~~~~~~~~~~~~~~~~~~~

To make a polar stereographic projection basemap with radius = 12 cm to
-60 degree latitude, with plot title "Salinity measurements", using 5
degrees annotation/tick interval and 1 degree gridlines, run

   ::

    gmt psbasemap -R-45/45/-90/-60 -Js0/-90/12c/-60 -B5g1 -B+t"Salinity measurements" > stereo1.ps

To make a 12-cm-wide stereographic basemap for Australia from an
arbitrary view point (not the poles), and use a rectangular boundary, we
must give the pole for the new projection and use the **-R** option to
indicate the lower left and upper right corners (in lon/lat) that will
define our rectangle. We choose a pole at 130/-30 and use 100/-45 and
160/-5 as our corners. The command becomes

   ::

    gmt psbasemap -R100/-45/160/-5r -JS130/-30/12c -Bafg -B+t"General Stereographic View" -P > stereo2.ps

`Miscellaneous Map Projections <#toc33>`_
-----------------------------------------

Hammer [equal-area]
~~~~~~~~~~~~~~~~~~~

The Hammer projection is mostly used for global maps and thus the
spherical form is used. To get a world map centered on Greenwich at a
scale of 1:200000000, use

   ::

    gmt psbasemap -Rd -Jh0/1:200000000 -Bafg -B+tHammer -P > hammer.ps

Sinusoidal [equal-area]
~~~~~~~~~~~~~~~~~~~~~~~

To make a sinusoidal world map centered on Greenwich, with a scale along
the equator of 0.02 inch/degree, use

   ::

    gmt psbasemap -Rd -Ji0/0.02i -Bafg -B+tSinusoidal -P > sinus1.ps

To make an interrupted sinusoidal world map with breaks at 160W, 20W,
and 60E, with a scale along the equator of 0.02 inch/degree, run the
following sequence of commands:

   ::

    gmt psbasemap -R-160/-20/-90/90 -Ji-90/0.02i -Bx30g30 -By15g15 -BWesn -K -P > sinus_i.ps
    gmt psbasemap -R-20/60/-90/90 -Ji20/0.02i -Bx30g30 -By15g15 -Bwesn -O -K -X2.8i >> sinus_i.ps
    gmt psbasemap -R60/200/-90/90 -Ji130/0.02i -Bx30g30 -By15g15 -BwEsn -O -X1.6i >> sinus_i.ps

Eckert IV [equal-area]
~~~~~~~~~~~~~~~~~~~~~~

Pseudo-cylindrical projection typically used for global maps only. Set
the central longitude and scale, e.g.,

   ::

    gmt psbasemap -Rg -Jkf180/0.064c -Bafg -B+t"Eckert IV" -P > eckert4.ps

Eckert VI [equal-area]
~~~~~~~~~~~~~~~~~~~~~~

Another pseudo-cylindrical projection typically used for global maps
only. Set the central longitude and scale, e.g.,

   ::

    gmt psbasemap -Rg -Jks180/0.064c -Bafg -B+t"Eckert VI" -P > eckert6.ps

Robinson
~~~~~~~~

Projection designed to make global maps "look right". Set the central
longitude and width, e.g.,

   ::

    gmt psbasemap -Rd -JN0/8i -Bafg -B+tRobinson -P > robinson.ps

Winkel Tripel
~~~~~~~~~~~~~

Yet another projection typically used for global maps only. You can set
the central longitude, e.g.,

   ::

    gmt psbasemap -R90/450/-90/90 -JR270/25c -Bafg -B+t"Winkel Tripel" -P > winkel.ps

Mollweide [equal-area]
~~~~~~~~~~~~~~~~~~~~~~

The Mollweide projection is also mostly used for global maps and thus
the spherical form is used. To get a 25-cm-wide world map centered on
the Dateline:

   ::

    psbasemap -Rg -JW180/25c -Bafg -B+tMollweide -P > mollweide.ps

Van der Grinten
~~~~~~~~~~~~~~~

The Van der Grinten projection is also mostly used for global maps and
thus the spherical form is used. To get a 18-cm-wide world map centered on the Dateline:

   ::

    gmt psbasemap -Rg -JV180/18c -Bafg -B+t"Van der Grinten" -P > grinten.ps

Arbitrary rotation
~~~~~~~~~~~~~~~~~~

If you need to plot a map but have it rotated about a vertical axis then
use the **-p** option.  For instance, the rotate the basemap below 90
degrees about an axis centered on the map, try

   ::

    gmt psbasemap -R10/40/10/40 -JM10c -P -Bafg -B+t"I am rotated" -p90+w25/25 -Xc -P > rotated.ps

.. include:: basemap_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`, :doc:`gmtcolors`
