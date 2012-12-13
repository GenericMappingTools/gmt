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
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
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
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]
    *west*, *east*, *south*, and *north* specify the region of interest,
    and you may specify them in decimal degrees or in
    [+-]dd:mm[:ss.xxx][W\|E\|S\|N] format. Append **r** if lower left
    and upper right map coordinates are given instead of w/e/s/n. The
    two shorthands **-Rg** and **-Rd** stand for global domain (0/360
    and -180/+180 in longitude respectively, with -90/+90 in latitude).
    Alternatively, specify the name of an existing grid file and the
    **-R** settings (and grid spacing, if applicable) are copied from
    the grid. Using **-R**\ *unit* expects projected (Cartesian)
    coordinates compatible with chosen **-J** and we inversely project
    to determine actual rectangular geographic region.
    For perspective view (**-p**), optionally append /*zmin*/*zmax*.

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ *min\_area*\ [/*min\_level*/*max\_level*][\ **+r**\ \|\ **l**][\ **p**\ *percent*]
    Features with an area smaller than *min\_area* in km^2 or of
    hierarchical level that is lower than *min\_level* or higher than
    *max\_level* will not be plotted [Default is 0/0/4 (all features)].
    Level 2 (lakes) contains regular lakes and wide river bodies which
    we normally include as lakes; append **+r** to just get river-lakes
    or **+l** to just get regular lakes. Finally, append
    **+p**\ *percent* to exclude polygons whose percentage area of the
    corresponding full-resolution feature is less than *percent*. See
    GSHHS INFORMATION below for more details.
**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
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

**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-Q**
    Mark end of existing clip path. No projection information is needed.
    Also supply **-X** and **-Y** settings if you have moved since the
    clip started.
**-S**\ *fill*\ \|\ **c**
    Select filling or clipping of "wet" areas. Append the shade, color,
    or pattern; or use **-Sc** for clipping [Default is no fill].
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
**-W**\ [*level*/]*pen*
    Draw shorelines [Default is no shorelines]. Append pen attributes
    [Defaults: width = default, color = black, style = solid] which
    apply to all four levels. To set the pen for each level differently,
    prepend *level*/, where *level* is 1-4 and represent coastline,
    lakeshore, island-in-lake shore, and lake-in-island-in-lake shore.
    Repeat **-W** as needed. When specific level pens are set, those not
    listed will not be drawn [Default draws all levels; but see **-A**].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output.
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
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

`Gshhs Information <#toc7>`_
----------------------------

The coastline database is GSHHS which is compiled from two sources:
World Vector Shorelines (WVS) and CIA World Data Bank II (WDBII). In
particular, all level-1 polygons (ocean-land boundary) are derived from
the more accurate WVS while all higher level polygons (level 2-4,
representing land/lake, lake/island-in-lake, and
island-in-lake/lake-in-island-in-lake boundaries) are taken from WDBII.
Much processing has taken place to convert WVS and WDBII data into
usable form for **GMT**: assembling closed polygons from line segments,
checking for duplicates, and correcting for crossings between polygons.
The area of each polygon has been determined so that the user may choose
not to draw features smaller than a minimum area (see **-A**); one may
also limit the highest hierarchical level of polygons to be included (4
is the maximum). The 4 lower-resolution databases were derived from the
full resolution database using the Douglas-Peucker line-simplification
algorithm. The classification of rivers and borders follow that of the
WDBII. See the **GMT** Cookbook and Technical Reference Appendix K for
further details.

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

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*gmtcolors*\ (5) <gmtcolors.html>`_ ,
`*grdlandmask*\ (1) <grdlandmask.html>`_ ,
`*psbasemap*\ (1) <psbasemap.html>`_
