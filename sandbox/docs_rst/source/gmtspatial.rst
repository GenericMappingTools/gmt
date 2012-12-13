**********
gmtspatial
**********

gmtspatial - Do geospatial operations on lines and polygons

`Synopsis <#toc1>`_
-------------------

**gmtspatial** [ *table* ] [ **-C** ] [
**-D**\ [**+f**\ *file*][\ **+a**\ *amax*][\ **+d**\ *dmax*][\ **+c\|C**\ *cmax*][\ **+s**\ *fact*]
] [ **-E**\ **+**\ \|\ **-** ] [ **-I**\ [**e**\ \|\ **i**] ] [
**-N**\ *pfile*\ [**+a**\ ][\ **+p**\ *start*][**+r**\ ][**+z**\ ] ] [
**-Q**\ [**+**\ [*unit*\ ]] ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [
**-S**\ **i**\ \|\ **u**\ \|\ **s**\ \|\ **j** ] [
**-T**\ [*clippolygon*\ ] ] [[ **-V**\ [*level*\ ] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-bo**\ [*ncols*\ ][*type*\ ] ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo*
] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**gmtspatial** reads one or more data files (which may be multisegment
files) that contains closed polygons and operates of these polygons in
the specified way. Operations include area calculation, handedness
reversals, and polygon intersections.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

None.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    data table file(s) holding a number of data columns. If no tables
    are given then we read from standard input.
**-C**
    Clips polygons to the map region, including map boundary to the
    polygon as needed. The result is a closed polygon (see **-T** for
    truncation instead). Requires **-R**.
**-D**\ [**+f**\ *file*][\ **+a**\ *amax*][\ **+d**\ *dmax*][\ **+c\|C**\ *cmax*][\ **+s**\ *fact*]
    Check for duplicates among the input lines or polygons, or, if
    *file* is given via **+f**, check if the input features already
    exist among the features in *file*. We consider the cases of exact
    (same number and coordinates) and approximate matches (average
    distance between nearest points of two features is less than a
    threshold). We also consider that some features may have been
    reversed. Features are considered approximate matches if their
    minimum distance is less than *dmax* [0] (see UNITS) and their
    closeness (defined as the ratio between the average distance between
    the features divided by their average length) is less than *cmax*
    [0.01]. For each duplicate found, the output record begins with the
    single letter Y (exact match) or ~ (approximate match). If the two
    matching segments differ in length by more than a factor of 2 then
    we consider the duplicate to be either a subset (-) or a superset
    (+). For polygons we also consider the fractional difference in
    areas; duplicates must differ by less than *amax* [0.01]. By
    default, we compute the mean line separation. Use **-+C**\ *cmin* to
    instead compute the median line separation and therefore a robust
    closeness value. Also by default we consider all distances between
    points on one line and another. Append **-+p** to limit the
    comparison to points that project perpendicularly to points on the
    other line (and not its extension).
**-E**\ **+**\ \|\ **-** ]
    Reset the handedness of all polygons to match the given **+**
    (counter-clockwise) or **-** (clockwise). Implies **-Q+**.
**-I**\ [**e**\ \|\ **i**]
    Determine the intersection locations between all pairs of polygons.
    Append **i** to only compute internal (i.e., self-intersecting
    polygons) crossovers or **e** to only compute external (i.e.,
    between paris of polygons) crossovers [Default is both].
**-N**\ *pfile*\ [**+a**\ ][\ **+p**\ *start*][**+r**\ ][**+z**\ ]
    Determine if one (or all, with **+a**) points of each feature in the
    input data are inside any of the polygons given in the *pfile*. If
    inside, then report which polygon it is; the polygon ID is either
    taken from the aspatial value assigned to Z, the segment header
    (first **-Z**, then **-L** are scanned), or it is assigned the
    running number that is initialized to *start* [0]. By default the
    input segment that are found to be inside a polygon are written to
    stdout with the polygon ID encoded in the segment header as
    **-Z**\ *ID*. Alternatively, append **+r** to just report which
    polygon contains a feature or **+z** to have the IDs added as an
    extra data column on output. Segments that fail to be inside a
    polygon are not written out. If more than one polygon contains the
    same segment we skip the second (and further) scenario.
**-Q**\ [**+**\ [*unit*\ ]]
    Measure the area of all polygons or length of line segments. Use
    **-Q+** to append the area to each polygons segment header [Default
    simply writes the area to stdout]. For polygons we also compute the
    centroid location while for line data we compute the mid-point
    (half-length) position. Append a distance unit to select the unit
    used (see UNITS). Note that the area will depend on the current
    setting of PROJ\_ELLIPSOID; this should be a recent ellipsoid to get
    accurate results.
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
    to determine actual rectangular geographic region. Clips polygons to
    the map region, including map boundary to the polygon as needed. The
    result is a closed polygon.
**-S**\ **i**\ \|\ **u**\ \|\ **s**\ \|\ **j**
    Spatial processing of polygons. Choose from **-Si** which returns
    the intersection of polygons (closed), **-Sc** which returns the
    union of polygons (closed), **-Ss** which will split polygons that
    straddle the Dateline, and **-Sj** which will join polygons that
    were split by the Dateline.
**-T**\ [*clippolygon*\ ]
    Truncate polygons against the specified polygon given, possibly
    resulting in open polygons. If no argument is given to **-T** we
    create a clipping polygon from **-R** which then is required. Note
    that when the **-R** clipping is in effect we will also look for
    polygons of length 4 or 5 that exactly match the **-R** clipping
    polygon.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output. [Default is same as input].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
(\*)
    Determine data gaps and line breaks.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Units <#toc6>`_
----------------

For map distance unit, append *unit* **d** for arc degree, **m** for arc
minute, and **s** for arc second, or **e** for meter [Default], **f**
for foot, **k** for km, **M** for statute mile, **n** for nautical mile,
and **u** for US survey foot. By default we compute such distances using
a spherical approximation with great circles. Prepend **-** to a
distance (or the unit is no distance is given) to perform "Flat Earth"
calculations (quicker but less accurate) or prepend **+** to perform
exact geodesic calculations (slower but more accurate).

`Ascii Format Precision <#toc7>`_
---------------------------------

The ASCII output formats of numerical data are controlled by parameters
in your **gmt.conf** file. Longitude and latitude are formatted
according to **FORMAT\_GEO\_OUT**, whereas other values are formatted
according to **FORMAT\_FLOAT\_OUT**. Be aware that the format in effect
can lead to loss of precision in the output, which can lead to various
problems downstream. If you find the output is not written with enough
precision, consider switching to binary output (**-bo** if available) or
specify more decimals using the **FORMAT\_FLOAT\_OUT** setting.

`Example <#toc8>`_
------------------

To compute the area of all geographic polygons in the multisegment file
polygons.d, run

gmtspatial polygons.d -Q > areas.d

Same data, but now orient all polygons to go counter-clockwise and write
their areas to the segment headers, run

gmtspatial polygons.d -Q+ -E+ > areas.d

To determine the intersections between the polygons A.d and B.d, run

gmtspatial A.d B.d -Ce > crossovers.d

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_
