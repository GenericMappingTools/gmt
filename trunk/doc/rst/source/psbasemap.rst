.. index:: ! psbasemap

*********
psbasemap
*********

.. only:: not man

    psbasemap - Plot PostScript base maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**psbasemap** **-J**\ *parameters*
|SYN_OPT-Rz|
[ |SYN_OPT-B| ]
[ **-D**\ [*unit*]\ *xmin/xmax/ymin/ymax*\ [**r**]\ \|\ *width*\ [/*height*][**+c**\ *clon/clat*][**+p**\ *pen*][**+g**\ *fill*]]
[ **-K** ] [ **-Jz**\ \|\ **Z**\ *parameters* ]
**-L**\ [**f**][**x**]\ *lon0*/*lat0*\ [/*slon*]/\ *slat*/*length*\ [**e**\ \|\ **f**\ \|\ **k**\ \|\ **M**\ \|\ **n**\ \|\ **u**][\ **+l**\ *label*][\ **+j**\ *just*][\ **+p**\ *pen*][\ **+g**\ *fill*][**+u**] ] ]
[ **-O** ] [ **-P** ]
[ |SYN_OPT-U| ]
[ **-T**\ [**f**\ \|\ **m**][**x**\ ]\ *lon0*/*lat0*/*size*\ [/*info*][\ **:**\ *w*,\ *e*,\ *s*,\ *n*\ **:**][\ **+**\ *gint*\ [/*mint*]] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-c| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]

|No-spaces|

Description
-----------

**psbasemap** creates PostScript code that will produce a basemap.
Several map projections are available, and the user may specify separate
tickmark intervals for boundary annotation, ticking, and [optionally]
gridlines. A simple map scale or directional rose may also be plotted.
At least one of the options **-B**,
**-L**\ [**f**][**x**]\ *lon0*/*lat0*\ [/*slon*]/\ *slat*/*length*\ [**e**\ \|\ **f**\ \|\ **k**\ \|\ **M**\ \|\ **n**\ \|\ **u**][\ **+l**\ *label*][\ **+j**\ *just*][\ **+p**\ *pen*][\ **+g**\ *fill*][**+u**]
], or
**-T**\ [**f**\ \|\ **m**][**x**]\ *lon0*/*lat0*/*size*\ [/*info*][\ **:**\ *w*,\ *e*,\ *s*,\ *n*\ **:**][\ **+**\ *gint*\ [/*mint*]]
must be specified. 

Required Arguments
------------------

.. include:: explain_-J.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

Optional Arguments
------------------

.. include:: explain_-B.rst_

[**-D**\ [*unit*]\ *xmin/xmax/ymin/ymax*\ [**r**]\ \|\ *width*\ [/*height*][**+c**\ *clon/clat*][**+p**\ *pen*][**+g**\ *fill*]]
    Draw a simple map insert box on the map.  Specify the box in one of three ways:
    (a) Give *west/east/south/north* of geographic rectangle bounded by parallels
    and meridians; append **r** if the coordinates instead are the lower left and
    upper right corners of the desired rectangle. (b) Give **u**\ *xmin/xmax/ymin/ymax*
    of bounding rectangle in projected coordinates (here, **u** is the coordinate unit).
    (c) Give [**u**]\ *width*\ [/*height*] of bounding rectangle and use **+c** to set
    box center. Append any combination of the following modifiers to draw the insert box:
    **+c**\ *lon/lat* to specify box center.
    **+g**\ *fill* to paint a insert [no fill].
    **+p**\ *pen* to draw the insert outline [no outline].

.. include:: explain_-Jz.rst_

.. include:: explain_-K.rst_

.. include:: explain_-L_scale.rst_

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

.. include:: explain_-T_rose.rst_

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_-XY.rst_

.. include:: explain_-c.rst_

.. |Add_-f| replace:: This applies only to the coordinates specified in the **-R** option. 
.. include:: explain_-f.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

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

    gmt psbasemap -R0/9/0/5 -Jx1 -Bf1a2:Distance:/:"No of samples":WeSn > linear.ps

Log-log plot
~~~~~~~~~~~~

To make a log-log frame with only the left and bottom axes, where the
x-axis is 25 cm and annotated every 1-2-5 and the y-axis is 15 cm and
annotated every power of 10 but has tickmarks every 0.1, run

   ::

    gmt psbasemap -R1/10000/1e20/1e25 -JX25cl/15cl -B2:Wavelength:/a1pf3:Power:WS > loglog.ps

Power axes
~~~~~~~~~~

To design an axis system to be used for a depth-sqrt(age) plot with
depth positive down, ticked and annotated every 500m, and ages annotated
at 1 my, 4 my, 9 my etc, use

   ::

    gmt psbasemap -R0/100/0/5000 -Jx1p0.5/-0.001 -B1p:"Crustal age":/500:Depth: > power.ps

Polar (theta,r) plot
~~~~~~~~~~~~~~~~~~~~

For a base map for use with polar coordinates, where the radius from 0
to 1000 should correspond to 3 inch and with gridlines and ticks every
30 degrees and 100 units, use

   ::

    gmt psbasemap -R0/360/0/1000 -JP6i -B30p/100 > polar.ps

Cylindrical Map Projections
---------------------------

Cassini
~~~~~~~

A 10-cm-wide basemap using the Cassini projection may be obtained by

   ::

    gmt psbasemap -R20/50/20/35 -JC35/28/10c -P -B5g5:.Cassini: > cassini.ps

Mercator [conformal]
~~~~~~~~~~~~~~~~~~~~

A Mercator map with scale 0.025 inch/degree along equator, and showing
the length of 5000 km along the equator (centered on 1/1 inch), may be
plotted as

   ::

    gmt psbasemap -R90/180/-50/50 -Jm0.025i -B30g30:.Mercator: -Lx1i/1i/0/5000 > mercator.ps

Miller
~~~~~~

A global Miller cylindrical map with scale 1:200,000,000 may be plotted as

   ::

    gmt psbasemap -Rg -Jj180/1:200000000 -B30g30:.Miller: > miller.ps

Oblique Mercator [conformal]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To create a page-size global oblique Mercator basemap for a pole at
(90,30) with gridlines every 30 degrees, run

   ::

    gmt psbasemap -R0/360/-70/70 -Joc0/0/90/30/0.064cd -B30g30:."Oblique Mercator": > oblmerc.ps

Transverse Mercator [conformal]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A regular Transverse Mercator basemap for some region may look like

   ::

    gmt psbasemap -R69:30/71:45/-17/-15:15 -Jt70/1:1000000 -B15m:."Survey area": -P > transmerc.ps

Equidistant Cylindrical Projection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This projection only needs the central meridian and scale. A 25 cm wide
global basemap centered on the 130E meridian is made by

   ::

    gmt psbasemap -R-50/310/-90/90 -JQ130/25c -B30g30:."Equidistant Cylindrical": > cyl\_eqdist.ps

Universal Transverse Mercator [conformal]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To use this projection you must know the UTM zone number, which defines
the central meridian. A UTM basemap for Indo-China can be plotted as

   ::

    gmt psbasemap -R95/5/108/20r -Ju46/1:10000000 -B3g3:.UTM: > utm.ps

Cylindrical Equal-Area
~~~~~~~~~~~~~~~~~~~~~~

First select which of the cylindrical equal-area projections you want by
deciding on the standard parallel. Here we will use 45 degrees which
gives the Gall-Peters projection. A 9 inch wide global basemap centered
on the Pacific is made by

   ::

    gmt psbasemap -Rg -JY180/45/9i -B30g30:.Gall-Peters: > gall-peters.ps

Conic Map Projections
---------------------

Albers [equal-area]
~~~~~~~~~~~~~~~~~~~

A basemap for middle Europe may be created by

   ::

    gmt psbasemap -R0/90/25/55 -Jb45/20/32/45/0.25c -B10g10:."Albers Equal-area": > albers.ps

Lambert [conformal]
~~~~~~~~~~~~~~~~~~~

Another basemap for middle Europe may be created by

   ::

    gmt psbasemap -R0/90/25/55 -Jl45/20/32/45/0.1i -B10g10:."Lambert Conformal Conic": > lambertc.ps

Equidistant
~~~~~~~~~~~

Yet another basemap of width 6 inch for middle Europe may be created by

   ::

    gmt psbasemap -R0/90/25/55 -JD45/20/32/45/6i -B10g10:."Equidistant conic": > econic.ps

Polyconic
~~~~~~~~~

A basemap for north America may be created by

   ::

    gmt psbasemap -R-180/-20/0/90 -JPoly/4i -B30g10/10g10:."Polyconic": > polyconic.ps

Azimuthal Map Projections
-------------------------

Lambert [equal-area]
~~~~~~~~~~~~~~~~~~~~

A 15-cm-wide global view of the world from the vantage point -80/-30
will give the following basemap:

   ::

    gmt psbasemap -Rg -JA-80/-30/15c -B30g30/15g15:."Lambert Azimuthal": > lamberta.ps

Follow the instructions for stereographic projection if you want to
impose rectangular boundaries on the azimuthal equal-area map but
substitute **-Ja** for **-Js**.

Equidistant
~~~~~~~~~~~

A 15-cm-wide global map in which distances from the center (here 125/10)
to any point is true can be obtained by:

   ::

    gmt psbasemap -Rg -JE125/10/15c -B30g30/15g15:.Equidistant: > equi.ps

Gnomonic
~~~~~~~~

A view of the world from the vantage point -100/40 out to a horizon of
60 degrees from the center can be made using the Gnomonic projection:

   ::

    gmt psbasemap -Rg -JF-100/40/60/6i -B30g30/15g15:.Gnomonic: > gnomonic.ps

Orthographic
~~~~~~~~~~~~

A global perspective (from infinite distance) view of the world from the
vantage point 125/10 will give the following 6-inch-wide basemap:

   ::

    gmt psbasemap -Rg -JG125/10/6i -B30g30/15g15:.Orthographic: > ortho.ps

General Perspective
~~~~~~~~~~~~~~~~~~~

The **-JG** option can be used in a more generalized form, specifying
altitude above the surface, width and height of the view point, and
twist and tilt. A view from 160 km above -74/41.5 with a tilt of 55 and
azimuth of 210 degrees, and limiting the viewpoint to 30 degrees width
and height will product a 6-inch-wide basemap:

   ::

    gmt psbasemap -Rg -JG-74/41.5/160/210/55/30/30/6i -B5g1/5g1:."General Perspective": > genper.ps

Stereographic [conformal]
~~~~~~~~~~~~~~~~~~~~~~~~~

To make a polar stereographic projection basemap with radius = 12 cm to
-60 degree latitude, with plot title "Salinity measurements", using 5
degrees annotation/tick interval and 1 degree gridlines, run

   ::

    gmt psbasemap -R-45/45/-90/-60 -Js0/-90/12c/-60 -B5g5:."Salinity measurements": > stereo1.ps

To make a 12-cm-wide stereographic basemap for Australia from an
arbitrary view point (not the poles), and use a rectangular boundary, we
must give the pole for the new projection and use the **-R** option to
indicate the lower left and upper right corners (in lon/lat) that will
define our rectangle. We choose a pole at 130/-30 and use 100/-45 and
160/-5 as our corners. The command becomes

   ::

    gmt psbasemap -R100/-45/160/-5r -JS130/-30/12c -B30g30/15g15:."General Stereographic View": > stereo2.ps

`Miscellaneous Map Projections <#toc33>`_
-----------------------------------------

Hammer [equal-area]
~~~~~~~~~~~~~~~~~~~

The Hammer projection is mostly used for global maps and thus the
spherical form is used. To get a world map centered on Greenwich at a
scale of 1:200000000, use

   ::

    gmt psbasemap -Rd -Jh0/1:200000000 -B30g30/15g15:.Hammer: > hammer.ps

Sinusoidal [equal-area]
~~~~~~~~~~~~~~~~~~~~~~~

To make a sinusoidal world map centered on Greenwich, with a scale along
the equator of 0.02 inch/degree, use

   ::

    gmt psbasemap -Rd -Ji0/0.02i -B30g30/15g15:.Sinusoidal: > sinus1.ps

To make an interrupted sinusoidal world map with breaks at 160W, 20W,
and 60E, with a scale along the equator of 0.02 inch/degree, run the
following sequence of commands:

   ::

    gmt psbasemap -R-160/-20/-90/90 -Ji-90/0.02i -B30g30/15g15Wesn -K > sinus_i.ps
    gmt psbasemap -R-20/60/-90/90 -Ji20/0.02i -B30g30/15g15wesn -O -K -X2.8i >> sinus_i.ps
    gmt psbasemap -R60/200/-90/90 -Ji130/0.02i -B30g30/15g15wEsn -O -X1.6i >> sinus_i.ps

Eckert IV [equal-area]
~~~~~~~~~~~~~~~~~~~~~~

Pseudo-cylindrical projection typically used for global maps only. Set
the central longitude and scale, e.g.,

   ::

    gmt psbasemap -Rg -Jkf180/0.064c -B30g30/15g15:."Eckert IV": > eckert4.ps

Eckert VI [equal-area]
~~~~~~~~~~~~~~~~~~~~~~

Another pseudo-cylindrical projection typically used for global maps
only. Set the central longitude and scale, e.g.,

   ::

    gmt psbasemap -Rg -Jks180/0.064c -B30g30/15g15:."Eckert VI": > eckert6.ps

Robinson
~~~~~~~~

Projection designed to make global maps "look right". Set the central
longitude and width, e.g.,

   ::

    gmt psbasemap -Rd -JN0/8i -B30g30/15g15:.Robinson: > robinson.ps

Winkel Tripel
~~~~~~~~~~~~~

Yet another projection typically used for global maps only. You can set
the central longitude, e.g.,

   ::

    gmt psbasemap -R90/450/-90/90 -JR270/25c -B30g30/15g15:."Winkel Tripel": > winkel.ps

Mollweide [equal-area]
~~~~~~~~~~~~~~~~~~~~~~

The Mollweide projection is also mostly used for global maps and thus
the spherical form is used. To get a 25-cm-wide world map centered on
the Dateline:

   ::

    psbasemap -Rg -JW180/25c -B30g30/15g15:.Mollweide: > mollweide.ps

Van der Grinten
~~~~~~~~~~~~~~~

The Van der Grinten projection is also mostly used for global maps and
thus the spherical form is used. To get a 7-inch-wide world map centered on the Dateline:

   ::

    gmt psbasemap -Rg -JV180/7i -B30g30/15g15:."Van der Grinten": > grinten.ps

CUSTOM lABELS OR INTERVALS
--------------------------

The **-B** option sets up a regular annotation interval and the
annotations derive from the corresponding *x*, *y*, or *z* coordinates.
However, some applications requires special control on which annotations
to plot and even replace the annotation with other labels. This is
achieved by using **c**\ *intfile* in the **-B** option, where *intfile*
contains all the information about annotations, ticks, and even
gridlines. Each record is of the form *coord* *type* [*label*\ ], where
*coord* is the coordinate for this annotation (or tick or gridline),
*type* is one or more letters from **a** (annotation), **i** interval
annotation, **f** tickamrk, and **g** gridline. Note that **a** and
**i** are mutually exclusive and cannot both appear in the same
*intfile*. Both **a** and **i** requires you to supply a *label* which
is used as the plot annotation. If not given then a regular formatted
annotation based on the coordinate will occur.

Restrictions
------------

For some projections, a spherical earth is implicitly assumed. A warning
will notify the user if **-V** is set.

Bugs
----

The **-B** option is somewhat complicated to explain and comprehend.
However, it is fairly simple for most applications (see examples).

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`, :doc:`gmtcolors`
