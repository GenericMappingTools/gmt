.. index:: ! coast

*******
coast
*******

.. only:: not man

    Plot continents, shorelines, rivers, and borders on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt coast** |-J|\ *parameters*
|SYN_OPT-R|
[ |-A|\ *area* ]
[ |SYN_OPT-B| ]
[ |-C|\ [**l**\ \|\ **r**/]\ *fill* ]
[ |-D|\ *resolution*\ [**+**] ]
[ |-E|\ *dcw* ]
[ |-F|\ *box* ]
[ |-G|\ *fill*\ \|\ **c** ]
[ |-I|\ *river*\ [/\ *pen*] ]
[ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-L|\ *scalebar* ]
[ |-M| ]
[ |-N|\ *border*\ [/*pen*] ]
[ |-Q| ]
[ |-S|\ *fill*\ \|\ **c** ]
[ |-T|\ *rose* ]
[ |-T|\ *mag_rose* ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*level*/]\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**coast** plots grayshaded, colored, or textured land-masses [or
water-masses] on maps and [optionally] draws coastlines, rivers, and
political boundaries. Alternatively, it can (1) issue clip
paths that will contain all land or all water areas, or
(2) dump the data to an ASCII table. The data files come
in 5 different resolutions: (**f**)ull, (**h**)igh, (**i**)ntermediate,
(**l**)ow, and (**c**)rude. The full resolution files amount to more
than 55 Mb of data and provide great detail; for maps of larger
geographical extent it is more economical to use one of the other
resolutions. If the user selects to paint the land-areas and does not
specify fill of water-areas then the latter will be transparent (i.e.,
earlier graphics drawn in those areas will not be overwritten).
Likewise, if the water-areas are painted and no land fill is set then
the land-areas will be transparent. A map projection must be supplied.

Required Arguments
------------------

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rgeo.rst_

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

Optional Arguments
------------------

.. _-A:

.. |Add_-A| unicode:: 0x20 .. just an invisible code
.. include:: explain_-A.rst_

.. _-B:

.. include:: explain_-B.rst_

.. _-C:

**-C**\ [**l**\ \|\ **r**/]\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Set the shade, color, or pattern for lakes and river-lakes [Default
    is the fill chosen for "wet" areas (**-S**)]. Optionally, specify
    separate fills by prepending **l/** for lakes and **r/** for
    river-lakes, repeating the **-C** option as needed.

.. _-D:

**-D**\ *resolution*\ [**+**]
    Selects the resolution of the data set to use ((**f**)ull,
    (**h**)igh, (**i**)ntermediate, (**l**)ow, and (**c**)rude). The
    resolution drops off by 80% between data sets [Default is **l**].
    Append **+** to automatically select a lower resolution should the one
    requested not be available [abort if not found].
    Alternatively, choose (**a**)uto to automatically select the best
    resolution given the chosen map scale.

.. _-E:

**-E**\ *code1,code2,...*\ [**+l**\|\ **L**][**+g**\ *fill*][**+p**\ *pen*]
    Select painting or dumping country polygons from the Digital Chart of the World.
    This is another dataset independent of GSHHG and hence the **-A** and **-D** options do not apply.
    Append one or more comma-separated countries using the 2-character
    ISO 3166-1 alpha-2 convention.  To select a state of a country
    (if available), append .state, e.g, US.TX for Texas.  To specify a
    whole continent, prepend = to any of the continent codes AF (Africa),
    AN (Antarctica), AS (Asia), EU (Europe), OC (Oceania),
    NA (North America), or SA (South America).  Append **+l** to
    just list the countries and their codes [no data extraction or plotting takes place].
    Use **+L** to see states/territories for Argentina, Australia, Brazil, Canada, and the US.
    Append **+p**\ *pen* to draw polygon outlines [no outline] and
    **+g**\ *fill* to fill them [no fill].  One of **+p**\|\ **g** must be
    specified unless **-M** is in effect, in which case only one **-E** option can be given.
    Otherwise, you may repeat **-E** to give different groups of items their own pen/fill settings.
    If neither **-J** nor **-M** are set then we just print the **-R**\ *wesn* string.

.. _-F:

**-F**\ [\ **+c**\ *clearances*][\ **+g**\ *fill*][**+i**\ [[*gap*/]\ *pen*]][\ **+p**\ [*pen*]][\ **+r**\ [*radius*\ ]][\ **+s**\ [[*dx*/*dy*/][*shade*\ ]]]
    Without further options, draws a rectangular border around the map scale or rose using
    :ref:`MAP_FRAME_PEN <MAP_FRAME_PEN>`; specify a different pen with **+p**\ *pen*.
    Add **+g**\ *fill* to fill the logo box [no fill].
    Append **+c**\ *clearance* where *clearance* is either *gap*, *xgap*\ /\ *ygap*,
    or *lgap*\ /\ *rgap*\ /\ *bgap*\ /\ *tgap* where these items are uniform, separate in
    x- and y-direction, or individual side spacings between logo and border. 
    Append **+i** to draw a secondary, inner border as well. We use a uniform
    *gap* between borders of 2\ **p** and the :ref:`MAP_DEFAULT_PEN <MAP_DEFAULT_PEN>`
    unless other values are specified. Append **+r** to draw rounded
    rectangular borders instead, with a 6\ **p** corner radius. You can
    override this radius by appending another value. Finally, append
    **+s** to draw an offset background shaded region. Here, *dx*/*dy*
    indicates the shift relative to the foreground frame
    [4\ **p**/-4\ **p**] and *shade* sets the fill style to use for shading [gray50].
    Requires **-L** or **-T**.  If both **-L** or **-T**, you may repeat **-F**
    after each of these.

.. _-G:

**-G**\ *fill*\ \|\ **c**
    Select filling or clipping of "dry" areas. Append the shade, color,
    or pattern; or use **-Gc** for clipping [Default is no fill].

.. _-I:

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

.. _-L:

.. include:: explain_-L_scale.rst_

.. _-M:

**-M**
    Dumps a single multisegment ASCII (or binary, see
    **-bo**) file to standard output. No plotting
    occurs. Specify one of **-E**, **-I**, **-N** or **-W**.
    Note: if **-M** is used with **-E** then **-R** or the **+r** modifier
    to **-E** are not required as we automatically determine the region
    given the selected geographic entities.

.. _-N:

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

.. _-Q:

**-Q**
    Mark end of existing clip path. No projection information is needed.
    Also supply **-X** and **-Y** settings if you have moved since the clip started.

.. _-S:

**-S**\ *fill*\ \|\ **c**
    Select filling or clipping of "wet" areas. Append the shade, color,
    or pattern; or use **-Sc** for clipping [Default is no fill].

.. _-T:

..  include:: explain_-T_rose.rst_

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [*level*/]\ *pen* :ref:`(more ...) <set-pens>`
    Draw shorelines [Default is no shorelines]. Append pen attributes
    [Defaults: width = default, color = black, style = solid] which
    apply to all four levels. To set the pen for each level differently,
    prepend *level*/, where *level* is 1-4 and represent coastline,
    lakeshore, island-in-lake shore, and lake-in-island-in-lake shore.
    Repeat **-W** as needed. When specific level pens are set, those not
    listed will not be drawn [Default draws all levels; but see **-A**].

.. _-X:

.. include:: explain_-XY.rst_

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bo.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

Examples
--------

To plot a green Africa with white outline on blue background, with
permanent major rivers in thick blue pen, additional major rivers in
thin blue pen, and national borders as dashed lines on a Mercator map at
scale 0.1 inch/degree, use

   ::

    gmt coast -R-30/30/-40/40 -Jm0.1i -B5 -I1/1p,blue -N1/0.25p,-
                -I2/0.25p,blue -W0.25p,white -Ggreen -Sblue -pdf africa

To plot Iceland using the lava pattern (# 28) at 100 dots per inch, on a
Mercator map at scale 1 cm/degree, run

   ::

    gmt coast -RIS+r1 -Jm1c -B -Wthin -Gp28+r100 -pdf iceland

To initiate a clip path for Africa so that the subsequent colorimage of
gridded topography is only seen over land, using a Mercator map at scale
0.1 inch/degree, use

   ::

    gmt begin
    gmt coast -R-30/30/-40/40 -Jm0.1i -B5 -Gc
    gmt grdimage etopo5.nc -Ccolors.cpt
    gmt coast -Q
    gmt end

To plot Great Britain, Italy, and France in blue with a red outline and
Spain, Portugal and Greece in yellow (no outline), and pick up the plot
domain form the extents of these countries, use

   ::

    gmt coast -JM6i -Baf -EGB,IT,FR+gblue+p0.25p,red -EES,PT,GR+gyellow -pdf map

To extract a high-resolution coastline data table for Iceland to be used
in your analysis, try

   ::

    gmt pscoast -RIS -Dh -W -M > iceland.txt

**coast** will first look for coastline files in directory
**$GMT_SHAREDIR**/coast If the desired file is not found, it will look
for the file **$GMT_SHAREDIR**/coastline.conf. This file may contain
any number of records that each holds the full pathname of an
alternative directory. Comment lines (#) and blank lines are allowed.
The desired file is then sought for in the alternate directories.

.. include:: explain_gshhs.rst_

Bugs
----

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
close enough may find that the GSHHG coastline is not matching other
data, e.g., satellite images, more recent coastline data, etc. We are
aware of such mismatches but cannot undertake band-aid solutions each
time this occurs.

Some users of **coast** will not be satisfied with what they find for
the Antarctic shoreline. In Antarctica, the boundary between ice and
ocean varies seasonally and inter-annually. There are some areas of
permanent shelf ice. In addition to these time-varying ice-ocean
boundaries, there are also shelf ice grounding lines where ice goes from
floating on the sea to sitting on land, and lines delimiting areas of
rock outcrop. For consistency's sake, we have used the World Vector
Shoreline throughout the world in **coast**, as described in the GMT
Cookbook Appendix K. Users who need specific boundaries in Antarctica
should get the Antarctic Digital Database, prepared by the British
Antarctic Survey, Scott Polar Research Institute, World Conservation
Monitoring Centre, under the auspices of the Scientific Committee on
Antarctic Research. This data base contains various kinds of limiting
lines for Antarctica and is available on CD-ROM. It is published by the
Scientific Committee on Antarctic Research, Scott Polar Research
Institute, Lensfield Road, Cambridge CB2 1ER, United Kingdom.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`grdlandmask`,
:doc:`basemap`
