*********
psbasemap
*********

psbasemap - Plot *PostScript* base maps

`Synopsis <#toc1>`_
-------------------

**psbasemap** **-J**\ *parameters*
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [
**-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [
**-L**\ [**f**\ ][**x**\ ]\ *lon0*/*lat0*\ [/*slon*]/\ *slat*/*length*\ [**e**\ \|\ **f**\ \|\ **k**\ \|\ **M**\ \|\ **n**\ \|\ **u**][\ **+l**\ *label*][\ **+j**\ *just*][\ **+p**\ *pen*][\ **+f**\ *fill*][**+u**\ ]
] ] [ **-O** ] [ **-P** ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [
**-T**\ [**f**\ \|\ **m**][**x**\ ]\ *lon0*/*lat0*/*size*\ [/*info*][\ **:**\ *w*,\ *e*,\ *s*,\ *n*\ **:**][\ **+**\ *gint*\ [/*mint*]]
] [ **-V**\ [*level*\ ] ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-c**\ *copies* ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**psbasemap** creates *PostScript* code that will produce a basemap.
Several map projections are available, and the user may specify separate
tickmark intervals for boundary annotation, ticking, and [optionally]
gridlines. A simple map scale or directional rose may also be plotted.
At least one of the options **-B**,
**-L**\ [**f**\ ][**x**\ ]\ *lon0*/*lat0*\ [/*slon*]/\ *slat*/*length*\ [**e**\ \|\ **f**\ \|\ **k**\ \|\ **M**\ \|\ **n**\ \|\ **u**][\ **+l**\ *label*][\ **+j**\ *just*][\ **+p**\ *pen*][\ **+f**\ *fill*][**+u**\ ]
], or
**-T**\ [**f**\ \|\ **m**][**x**\ ]\ *lon0*/*lat0*/*size*\ [/*info*][\ **:**\ *w*,\ *e*,\ *s*,\ *n*\ **:**][\ **+**\ *gint*\ [/*mint*]]
must be specified.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-J**\ *parameters* (\*)
    Select map projection.
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
    For perspective view (**-p**), optionally append /*zmin*/*zmax*.

`Optional Arguments <#toc5>`_
-----------------------------

**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
**-Jz**\ \|\ **Z**\ *parameters* (\*)
    Set z-axis scaling; same syntax as **-Jx**.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-L**\ [**f**\ ][**x**\ ]\ *lon0*/*lat0*\ [/*slon*]/\ *slat*/*length*\ [**e**\ \|\ **f**\ \|\ **k**\ \|\ **M**\ \|\ **n**\ \|\ **u**][\ **+l**\ *label*][\ **+j**\ *just*][\ **+p**\ *pen*][\ **+f**\ *fill*][**+u**\ ]
]
    Draws a simple map scale centered on *lon0/lat0*. Use **-Lx** to
    specify x/y position instead. Scale is calculated at latitude *slat*
    (optionally supply longitude *slon* for oblique projections [Default
    is central meridian]), *length* is in km, or append unit from
    **e**\ \|\ **f**\ \|\ **k**\ \|\ **M**\ \|\ **n**\ \|\ **u**. Use
    **-Lf** to get a "fancy" scale [Default is plain]. Append **+l** to
    select the default label which equals the distance unit (meter,
    foot, km, mile, nautical mile, US survey foot) and is justified on
    top of the scale [t]. Change this by giving your own label (append
    **+l**\ *label*). Change label justification with
    **+j**\ *justification* (choose among l(eft), r(ight),
    `t(op) <t.op.html>`_ , and `b(ottom) <b.ottom.html>`_ ). Apply
    **+u** to append the unit to all distance annotations along the
    scale. If you want to place a rectangle behind the scale, specify
    suitable **+p**\ *pen* and/or **+f**\ *fill* parameters.
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-T**\ [**f**\ \|\ **m**][**x**\ ]\ *lon0*/*lat0*/*size*\ [/*info*][\ **:**\ *w*,\ *e*,\ *s*,\ *n*\ **:**][\ **+**\ *gint*\ [/*mint*]]
    Draws a simple map directional rose centered on *lon0/lat0*. Use
    **-Tx** to specify x/y position instead. The *size* is the diameter
    of the rose, and optional label information can be specified to
    override the default values of W, E, S, and N (Give **::** to
    suppress all labels). The default [plain] map rose only labels
    north. Use **-Tf** to get a "fancy" rose, and specify in *info* what
    you want drawn. The default [**1**\ ] draws the two principal E-W,
    N-S orientations, **2** adds the two intermediate NW-SE and NE-SW
    orientations, while **3** adds the eight minor orientations WNW-ESE,
    NNW-SSE, NNE-SSW, and ENE-WSW. For a magnetic compass rose, specify
    **-Tm**. If given, *info* must be the two parameters *dec/dlabel*,
    where *dec* is the magnetic declination and *dlabel* is a label for
    the magnetic compass needle (specify **-** to format a label from
    *dec*). Then, both directions to geographic and magnetic north are
    plotted [Default is geographic only]. If the north label is **\***
    then a north star is plotted instead of the north label. Annotation
    and two levels of tick intervals for both geographic and magnetic
    directions are 30/5/1 degrees; override these settings by appending
    **+**\ *gints*\ [/*mints*]. Color and pen attributes for the rose
    are taken from **COLOR\_BACKGROUND** and **MAP\_TICK\_PEN**,
    respectively, while label fonts, colors and sizes follow
    **FONT\_TITLE** for the four major directions and **FONT\_LABEL**
    for minor directions.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns. This applies only
    to the coordinates specified in the **-R** option.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
(\*)
    Select perspective view.
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Examples <#toc6>`_
-------------------

The following section illustrates the use of the options by giving some
examples for the available map projections. Note how scales may be given
in several different ways depending on the projection. Also note the use
of upper case letters to specify map width instead of map scale.

`Non-geographical Projections <#toc7>`_
---------------------------------------

`Linear x-y plot <#toc8>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~

To make a linear x/y frame with all axes, but with only left and bottom
axes annotated, using xscale = yscale = 1.0, ticking every 1 unit and
annotating every 2, and using xlabel = "Distance" and ylabel = "No of
samples", use

psbasemap -R0/9/0/5 -Jx1 -Bf1a2:Distance:/:"No of samples":WeSn >
linear.ps

`Log-log plot <#toc9>`_
~~~~~~~~~~~~~~~~~~~~~~~

To make a log-log frame with only the left and bottom axes, where the
x-axis is 25 cm and annotated every 1-2-5 and the y-axis is 15 cm and
annotated every power of 10 but has tickmarks every 0.1, run

psbasemap -R1/10000/1e20/1e25 **-JX**\ 25\ **cl**/15**cl**
-B2:Wavelength:/a1pf3:Power:WS > loglog.ps

`Power axes <#toc10>`_
~~~~~~~~~~~~~~~~~~~~~~

To design an axis system to be used for a depth-sqrt(age) plot with
depth positive down, ticked and annotated every 500m, and ages annotated
at 1 my, 4 my, 9 my etc, use

psbasemap -R0/100/0/5000 -Jx1p0.5/-0.001 -B1p:"Crustal age":/500:Depth:
> power.ps

`Polar (theta,r) plot <#toc11>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For a base map for use with polar coordinates, where the radius from 0
to 1000 should correspond to 3 inch and with gridlines and ticks every
30 degrees and 100 units, use

psbasemap -R0/360/0/1000 **-JP**\ 6\ **i** -B30p/100 > polar.ps

`Cylindrical Map Projections <#toc12>`_
---------------------------------------

`Cassini <#toc13>`_
~~~~~~~~~~~~~~~~~~~

A 10-cm-wide basemap using the Cassini projection may be obtained by

psbasemap -R20/50/20/35 **-JC**\ 35/28/10\ **c** -P -B5g5:.Cassini: >
cassini.ps

`Mercator [conformal] <#toc14>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A Mercator map with scale 0.025 inch/degree along equator, and showing
the length of 5000 km along the equator (centered on 1/1 inch), may be
plotted as

psbasemap -R90/180/-50/50 **-Jm**\ 0.025\ **i** -B30g30:.Mercator:
**-Lx**\ 1\ **i**/1**i**/0/5000 > mercator.ps

`Miller <#toc15>`_
~~~~~~~~~~~~~~~~~~

A global Miller cylindrical map with scale 1:200,000,000 may be plotted
as

psbasemap -Rg -Jj180/1:200000000 -B30g30:.Miller: > miller.ps

`Oblique Mercator [conformal] <#toc16>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To create a page-size global oblique Mercator basemap for a pole at
(90,30) with gridlines every 30 degrees, run

psbasemap -R0/360/-70/70 **-Joc**\ 0/0/90/30/0.064\ **c**\ d
-B30g30:."Oblique Mercator": > oblmerc.ps

`Transverse Mercator [conformal] <#toc17>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A regular Transverse Mercator basemap for some region may look like

psbasemap -R69:30/71:45/-17/-15:15 -Jt70/1:1000000 -B15m:."Survey area":
-P > transmerc.ps

`Equidistant Cylindrical Projection <#toc18>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This projection only needs the central meridian and scale. A 25 cm wide
global basemap centered on the 130E meridian is made by

psbasemap -R-50/310/-90/90 **-JQ**\ 130/25\ **c** -B30g30:."Equidistant
Cylindrical": > cyl\_eqdist.ps

`Universal Transverse Mercator [conformal] <#toc19>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To use this projection you must know the UTM zone number, which defines
the central meridian. A UTM basemap for Indo-China can be plotted as

psbasemap -R95/5/108/20r -Ju46/1:10000000 -B3g3:.UTM: > utm.ps

`Cylindrical Equal-Area <#toc20>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

First select which of the cylindrical equal-area projections you want by
deciding on the standard parallel. Here we will use 45 degrees which
gives the Gall-Peters projection. A 9 inch wide global basemap centered
on the Pacific is made by

psbasemap -Rg **-JY**\ 180/45/9\ **i** -B30g30:.Gall-Peters: >
gall-peters.ps

`Conic Map Projections <#toc21>`_
---------------------------------

`Albers [equal-area] <#toc22>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A basemap for middle Europe may be created by

psbasemap -R0/90/25/55 **-Jb**\ 45/20/32/45/0.25\ **c** -B10g10:."Albers
Equal-area": > albers.ps

`Lambert [conformal] <#toc23>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Another basemap for middle Europe may be created by

psbasemap -R0/90/25/55 **-Jl**\ 45/20/32/45/0.1\ **i** -B10g10:."Lambert
Conformal Conic": > lambertc.ps

`Equidistant <#toc24>`_
~~~~~~~~~~~~~~~~~~~~~~~

Yet another basemap of width 6 inch for middle Europe may be created by

psbasemap -R0/90/25/55 **-JD**\ 45/20/32/45/6\ **i**
-B10g10:."Equidistant conic": > econic.ps

`Polyconic <#toc25>`_
~~~~~~~~~~~~~~~~~~~~~

A basemap for north America may be created by

psbasemap -R-180/-20/0/90 **-JPoly**/4**i** -B30g10/10g10:."Polyconic":
> polyconic.ps

`Azimuthal Map Projections <#toc26>`_
-------------------------------------

`Lambert [equal-area] <#toc27>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A 15-cm-wide global view of the world from the vantage point -80/-30
will give the following basemap:

psbasemap -Rg **-JA**-80/-30/15\ **c** -B30g30/15g15:."Lambert
Azimuthal": > lamberta.ps

Follow the instructions for stereographic projection if you want to
impose rectangular boundaries on the azimuthal equal-area map but
substitute **-Ja** for **-Js**.

`Equidistant <#toc28>`_
~~~~~~~~~~~~~~~~~~~~~~~

A 15-cm-wide global map in which distances from the center (here 125/10)
to any point is true can be obtained by:

psbasemap -Rg **-JE**\ 125/10/15\ **c** -B30g30/15g15:.Equidistant: >
equi.ps

`Gnomonic <#toc29>`_
~~~~~~~~~~~~~~~~~~~~

A view of the world from the vantage point -100/40 out to a horizon of
60 degrees from the center can be made using the Gnomonic projection:

psbasemap -Rg **-JF**-100/40/60/6\ **i** -B30g30/15g15:.Gnomonic: >
gnomonic.ps

`Orthographic <#toc30>`_
~~~~~~~~~~~~~~~~~~~~~~~~

A global perspective (from infinite distance) view of the world from the
vantage point 125/10 will give the following 6-inch-wide basemap:

psbasemap -Rg **-JG**\ 125/10/6\ **i** -B30g30/15g15:.Orthographic: >
ortho.ps

`General Perspective <#toc31>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The **-JG** option can be used in a more generalized form, specifying
altitude above the surface, width and height of the view point, and
twist and tilt. A view from 160 km above -74/41.5 with a tilt of 55 and
azimuth of 210 degrees, and limiting the viewpoint to 30 degrees width
and height will product a 6-inch-wide basemap:

psbasemap -Rg **-JG**-74/41.5/160/210/55/30/30/6\ **i**
-B5g1/5g1:."General Perspective": > genper.ps

`Stereographic [conformal] <#toc32>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To make a polar stereographic projection basemap with radius = 12 cm to
-60 degree latitude, with plot title "Salinity measurements", using 5
degrees annotation/tick interval and 1 degree gridlines, run

psbasemap -R-45/45/-90/-60 **-Js**\ 0/-90/12\ **c**/-60 -B5g5:."Salinity
measurements": > stereo1.ps

To make a 12-cm-wide stereographic basemap for Australia from an
arbitrary view point (not the poles), and use a rectangular boundary, we
must give the pole for the new projection and use the **-R** option to
indicate the lower left and upper right corners (in lon/lat) that will
define our rectangle. We choose a pole at 130/-30 and use 100/-45 and
160/-5 as our corners. The command becomes

psbasemap -R100/-45/160/-5r **-JS**\ 130/-30/12\ **c**
-B30g30/15g15:."General Stereographic View": > stereo2.ps

`Miscellaneous Map Projections <#toc33>`_
-----------------------------------------

`Hammer [equal-area] <#toc34>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Hammer projection is mostly used for global maps and thus the
spherical form is used. To get a world map centered on Greenwich at a
scale of 1:200000000, use

psbasemap -Rd -Jh0/1:200000000 -B30g30/15g15:.Hammer: > hammer.ps

`Sinusoidal [equal-area] <#toc35>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To make a sinusoidal world map centered on Greenwich, with a scale along
the equator of 0.02 inch/degree, use

psbasemap -Rd **-Ji**\ 0/0.02\ **i** -B30g30/15g15:.Sinusoidal: >
sinus1.ps

To make an interrupted sinusoidal world map with breaks at 160W, 20W,
and 60E, with a scale along the equator of 0.02 inch/degree, run the
following sequence of commands:

psbasemap -R-160/-20/-90/90 -Ji-90/0.02i -B30g30/15g15Wesn -K >
sinus\_i.ps

psbasemap -R-20/60/-90/90 -Ji20/0.02i -B30g30/15g15wesn -O -K -X2.8i >>
sinus\_i.ps

psbasemap -R60/200/-90/90 -Ji130/0.02i -B30g30/15g15wEsn -O -X1.6i >>
sinus\_i.ps

`Eckert IV [equal-area] <#toc36>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Pseudo-cylindrical projection typically used for global maps only. Set
the central longitude and scale, e.g.,

psbasemap -Rg **-Jkf**\ 180/0.064\ **c** -B30g30/15g15:."Eckert IV": >
eckert4.ps

`Eckert VI [equal-area] <#toc37>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Another pseudo-cylindrical projection typically used for global maps
only. Set the central longitude and scale, e.g.,

psbasemap -Rg **-Jks**\ 180/0.064\ **c** -B30g30/15g15:."Eckert VI": >
eckert6.ps

`Robinson <#toc38>`_
~~~~~~~~~~~~~~~~~~~~

Projection designed to make global maps "look right". Set the central
longitude and width, e.g.,

psbasemap -Rd **-JN**\ 0/8\ **i** -B30g30/15g15:.Robinson: > robinson.ps

`Winkel Tripel <#toc39>`_
~~~~~~~~~~~~~~~~~~~~~~~~~

Yet another projection typically used for global maps only. You can set
the central longitude, e.g.,

psbasemap -R90/450/-90/90 **-JR**\ 270/25\ **c** -B30g30/15g15:."Winkel
Tripel": > winkel.ps

`Mollweide [equal-area] <#toc40>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Mollweide projection is also mostly used for global maps and thus
the spherical form is used. To get a 25-cm-wide world map centered on
the Dateline:

psbasemap -Rg **-JW**\ 180/25\ **c** -B30g30/15g15:.Mollweide: >
mollweide.ps

`Van der Grinten <#toc41>`_
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Van der Grinten projection is also mostly used for global maps and
thus the spherical form is used. To get a 7-inch-wide world map centered
on the Dateline:

psbasemap -Rg **-JV**\ 180/7\ **i** -B30g30/15g15:."Van der Grinten": >
grinten.ps

`CUSTOM lABELS OR INTERVALS <#toc42>`_
--------------------------------------

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

`Restrictions <#toc43>`_
------------------------

For some projections, a spherical earth is implicitly assumed. A warning
will notify the user if **-V** is set.

`Bugs <#toc44>`_
----------------

The **-B** option is somewhat complicated to explain and comprehend.
However, it is fairly simple for most applications (see examples).

`See Also <#toc45>`_
--------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*gmtcolors*\ (5) <gmtcolors.html>`_
