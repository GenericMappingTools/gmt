*******
pscoast
*******

pscoast - Plot continents, shorelines, rivers, and borders on maps

`Synopsis <#toc1>`_
-------------------

**pscoast** **-J**\ *parameters*
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] [
**-A**\ *min\_area*\ [/*min\_level*/*max\_level*][\ **+r**\ \|\ **l**][\ **p**\ *percent*]
] [ **-B**\ [**p**\ \|\ **s**]\ *parameters* ] [
**-C**\ [**l**\ \|\ **r**/]*fill* ] [ **-D**\ *resolution*\ [**+**\ ] ]
[ **-G**\ *fill*\ \|\ **c** ] [ **-I**\ *river*\ [/*pen*] ] [
**-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [
**-L**\ [**f**\ ][**x**\ ]\ *lon0*/*lat0*\ [/*slon*]/\ *slat*/*length*\ [**e**\ \|\ **f**\ \|\ **k**\ \|\ **M**\ \|\ **n**\ \|\ **u**][\ **+l**\ *label*][\ **+j**\ *just*][\ **+p**\ *pen*][\ **+f**\ *fill*][**+u**\ ]
] ] [ **-M** ] [ **-N**\ *border*\ [/*pen*] ] [ **-O** ] [ **-P** ] [
**-Q** ] [ **-S**\ *fill*\ \|\ **c** ] [
**-T**\ [**f**\ \|\ **m**][**x**\ ]\ *lon0*/*lat0*/*size*\ [/*info*][\ **:**\ *w*,\ *e*,\ *s*,\ *n*\ **:**][\ **+**\ *gint*\ [/*mint*]]
] [ **-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [
**-V**\ [*level*\ ] ] [ **-W**\ [*level*/]*pen* ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-bo**\ [*ncols*\ ][*type*\ ] ] [ **-c**\ *copies* ] [
**-p**\ [**x**\ \|\ **y**\ \|\ **z**]\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**pscoast** plots grayshaded, colored, or textured land-masses [or
water-masses] on maps and [optionally] draws coastlines, rivers, and
political boundaries. Alternatively, it `can (1) <can.html>`_ issue clip
paths that will contain all land or all water areas, `or
(2) <or.2.html>`_ dump the data to an ASCII table. The data files come
in 5 different resolutions: (**f**)ull, (**h**)igh, (**i**)ntermediate,
(**l**)ow, and (**c**)rude. The full resolution files amount to more
than 55 Mb of data and provide great detail; for maps of larger
geographical extent it is more economical to use one of the other
resolutions. If the user selects to paint the land-areas and does not
specify fill of water-areas then the latter will be transparent (i.e.,
earlier graphics drawn in those areas will not be overwritten).
Likewise, if the water-areas are painted and no land fill is set then
the land-areas will be transparent. A map projection must be supplied.
The *PostScript* code is written to standard output. 

.. include:: explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

.. include:: explain_-J.rst_

.. |Add_-Rgeo| unicode:: 0x0C .. just an invisible code
.. include:: explain_-Rgeo.rst_

.. |Add_-Rx| unicode:: 0x0C .. just an invisible code
.. include:: explain_-Rz.rst_

`Optional Arguments <#toc5>`_
-----------------------------

.. |Add_-A| unicode:: 0x0C .. just an invisible code
.. include:: explain_-A.rst_

.. include:: explain_-B.rst_

**-C**\ [**l**\ \|\ **r**/]*fill*
    Set the shade, color, or pattern for lakes and river-lakes [Default
    is the fill chosen for "wet" areas (**-S**)]. Optionally, specify
    separate fills by prepending **l**/ for lakes and **r**/ for
    river-lakes, repeating the **-C** option as needed.
**-D**\ *resolution*\ [**+**\ ]
    Selects the resolution of the data set to use ((**f**)ull,
    (**h**)igh, (**i**)ntermediate, (**l**)ow, and (**c**)rude). The
    resolution drops off by 80% between data sets [Default is **l**].
    Append )+) to automatically select a lower resolution should the one
    requested not be available [abort if not found].
**-G**\ *fill*\ \|\ **c**
    Select filling or clipping of "dry" areas. Append the shade, color,
    or pattern; or use **-Gc** for clipping [Default is no fill].
**-I**\ *river*\ [/*pen*]
    Draw rivers. Specify the type of rivers and [optionally] append pen
    attributes [Default pen: width = default, color = black, style =
    solid].

    Choose from the list of river types below; repeat option **-I** as
    often as necessary.

    0 = Double-lined rivers (river-lakes)

    1 = Permanent major rivers

    2 = Additional major rivers

    3 = Additional rivers

    4 = Minor rivers

    5 = Intermittent rivers - major

    6 = Intermittent rivers - additional

    7 = Intermittent rivers - minor

    8 = Major canals

    9 = Minor canals

    10 = Irrigation canals

    You can also choose from several preconfigured river groups:

    a = All rivers and canals (0-10)

    A = All rivers and canals except river-lakes (1-10)

    r = All permanent rivers (0-4)

    R = All permanent rivers except river-lakes (1-4)

    i = All intermittent rivers (5-7)

    c = All canals (8-10)

.. include:: explain_-Jz.rst_

.. include:: explain_-K.rst_

.. include:: explain_-L_scale.rst_

**-M**
    Dumps a single multisegment ASCII (or binary, see
    **-bo**\ [*ncols*\ ][*type*\ ]) file to standard output. No plotting
    occurs. Specify any combination of **-W**, **-I**, **-N**.
**-N**\ *border*\ [/*pen*]
    Draw political boundaries. Specify the type of boundary and
    [optionally] append pen attributes [Default pen: width = default,
    color = black, style = solid].

    Choose from the list of boundaries below. Repeat option **-N** as
    often as necessary.

    1 = National boundaries

    2 = State boundaries within the Americas

    3 = Marine boundaries

    a = All boundaries (1-3)

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

**-Q**
    Mark end of existing clip path. No projection information is needed.
    Also supply **-X** and **-Y** settings if you have moved since the
    clip started.
**-S**\ *fill*\ \|\ **c**
    Select filling or clipping of "wet" areas. Append the shade, color,
    or pattern; or use **-Sc** for clipping [Default is no fill]. 

..  include:: explain_-T_rose.rst_

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x0C .. just an invisible code
.. include:: explain_-V.rst_

**-W**\ [*level*/]*pen*
    Draw shorelines [Default is no shorelines]. Append pen attributes
    [Defaults: width = default, color = black, style = solid] which
    apply to all four levels. To set the pen for each level differently,
    prepend *level*/, where *level* is 1-4 and represent coastline,
    lakeshore, island-in-lake shore, and lake-in-island-in-lake shore.
    Repeat **-W** as needed. When specific level pens are set, those not
    listed will not be drawn [Default draws all levels; but see **-A**].
    
.. include:: explain_-XY.rst_

.. |Add_-bo| unicode:: 0x0C .. just an invisible code
.. include:: explain_-bo.rst_

.. include:: explain_-c.rst_

.. |Add_perspective| unicode:: 0x0C .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

`Examples <#toc6>`_
-------------------

To plot a green Africa with white outline on blue background, with
permanent major rivers in thick blue pen, additional major rivers in
thin blue pen, and national borders as dashed lines on a Mercator map at
scale 0.1 inch/degree, use

pscoast -R-30/30/-40/40 **-Jm**\ 0.1\ **i** -B5 -I1/1p,blue
-I2/0.25p,blue -N1/0.25p,- -W0.25p,white -Ggreen -Sblue -P > africa.ps

To plot Iceland using the lava pattern (# 28) at 100 dots per inch, on a
Mercator map at scale 1 cm/degree, run

pscoast -R-30/-10/60/65 **-Jm**\ 1\ **c** -B5 -Gp100/28 > iceland.ps

To initiate a clip path for Africa so that the subsequent colorimage of
gridded topography is only seen over land, using a Mercator map at scale
0.1 inch/degree, use

pscoast -R-30/30/-40/40 **-Jm**\ 0.1\ **i** -B5 -Gc -P -K > africa.ps

grdimage **-Jm**\ 0.1\ **i** etopo5.nc -Ccolors.cpt -O -K >> africa.ps

pscoast -Q -O >> africa.ps

**pscoast** will first look for coastline files in directory
**$GMT\_SHAREDIR**/coast If the desired file is not found, it will look
for the file **$GMT\_SHAREDIR**/coastline.conf. This file may contain
any number of records that each holds the full pathname of an
alternative directory. Comment lines (#) and blank lines are allowed.
The desired file is then sought for in the alternate directories. 

.. include:: explain_gshhs.rst_

`Bugs <#toc8>`_
---------------

The options to fill (**-C** **-G** **-S**) may not always work if the
Azimuthal equidistant projection is chosen (**-Je**\ \|\ **E**). If the
antipole of the projection is in the oceans it will most likely work. If
not, try to avoid using projection center coordinates that are even
multiples of the coastline bin size (1, 2, 5, 10, and 20 degrees for
**f**, **h**, **i**, **l**, **c**, respectively). This projection is not
supported for clipping.

The political borders are for the most part 1970s-style but have been
updated to reflect more recent border rearrangements in Europe and
elsewhere. Let us know if you find something out of date.

The full-resolution coastlines are also from a digitizing effort in the
1970-80s and it is difficult to assess the accuracy. Users who zoom in
close enough may find that the GSHHS coastline is not matching other
data, e.g., satellite images, more recent coastline data, etc. We are
aware of such mismatches but cannot undertake band-aid solutions each
time this occurs.

Some users of **pscoast** will not be satisfied with what they find for
the Antarctic shoreline. In Antarctica, the boundary between ice and
ocean varies seasonally and inter-annually. There are some areas of
permanent sea ice. In addition to these time-varying ice-ocean
boundaries, there are also ice grounding lines where ice goes from
floating on the sea to sitting on land, and lines delimiting areas of
rock outcrop. For consistency’s sake, we have used the World Vector
Shoreline throughout the world in pscoast, as described in the **GMT**
Cookbook Appendix K. Users who need specific boundaries in Antarctica
should get the Antarctic Digital Database, prepared by the British
Antarctic Survey, Scott Polar Research Institute, World Conservation
Monitoring Centre, under the auspices of the Scientific Committee on
Antarctic Research. This data base contains various kinds of limiting
lines for Antarctica and is available on CD-ROM. It is published by the
Scientific Committee on Antarctic Research, Scott Polar Research
Institute, Lensfield Road, Cambridge CB2 1ER, United Kingdom.

`See Also <#toc9>`_
-------------------

`gmt <gmt.html>`_ , `gmt.conf <gmt.conf.html>`_ ,
`gmtcolors <gmtcolors.html>`_ ,
`grdlandmask <grdlandmask.html>`_ ,
`psbasemap <psbasemap.html>`_
