.. index:: ! gmtspatial

*******
spatial
*******

.. only:: not man

    Geospatial operations on points, lines and polygons

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt spatial** [ *table* ] [ |-A|\ [**a**\ *min_dist*][*unit*]]
[ |-C| ]
[ |-D|\ [**+f**\ *file*][\ **+a**\ *amax*][\ **+d**\ *dmax*][\ **+c\|C**\ *cmax*][\ **+s**\ *fact*] ]
[ |-E|\ **+p**\ \|\ **n** ]
[ |-F|\ [**l**] ]
[ |-I|\ [**e**\ \|\ **i**] ]
[ |-N|\ *pfile*\ [**+a**][\ **+p**\ *start*][**+r**][**+z**] ]
[ |-Q|\ [*unit*\ ][**+c**\ *min*\ [/*max*]][**+h**\ ][**+l**\ ][**+p**\ ][**+s**\ [**a**\ \|\ **d**]] ]
[ |SYN_OPT-R| ]
[ |-S|\ **h**\ \|\ **i**\ \|\ **u**\ \|\ **s**\ \|\ **j** ]
[ |-T|\ [*clippolygon*] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-j| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**spatial** reads one or more data files (which may be multisegment
files) that contains closed polygons and operates of these polygons in
the specified way. Operations include area calculation, handedness
reversals, and polygon intersections. 

Required Arguments
------------------

None.

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-A:

**-A**\ [**a**\ *min_dist*][*unit*]
   Perform spatial nearest neighbor (NN) analysis: Determine the nearest
   neighbor of each point and report the NN distances and the point IDs
   involved in each pair (IDs are the input record numbers starting at 0).
   Use **-Aa** to decimate a data set so that no NN distance is lower than
   the threshold *min_dist*.  In this case we write out the (possibly
   averaged) coordinates and the updated NN distances and point IDs.  A
   negative point number means the original point was replaced by a weighted
   average (the absolute ID value gives the ID of the first original point
   ID to be included in the average.).  Note: The input data are assumed to
   contain (*lon, lat*) or (*x, y*), optionally followed by a *z* and a *weight* [1] column.
   We compute a weighted average of the location and *z* (if present).

.. _-C:

**-C**
    Clips polygons to the map region, including map boundary to the
    polygon as needed. The result is a closed polygon (see **-T** for
    truncation instead). Requires **-R**.

.. _-D:

**-D**\ [**+f**\ *file*][\ **+a**\ *amax*][\ **+d**\ *dmax*][\ **+c\|C**\ *cmax*][\ **+s**\ *fact*]
    Check for duplicates among the input lines or polygons, or, if
    *file* is given via **+f**, check if the input features already
    exist among the features in *file*. We consider the cases of exact
    (same number and coordinates) and approximate matches (average
    distance between nearest points of two features is less than a
    threshold). We also consider that some features may have been
    reversed. Features are considered approximate matches if their
    minimum distance is less than *dmax* [0] (see :ref:`Unit_attributes`) and their
    closeness (defined as the ratio between the average distance between
    the features divided by their average length) is less than *cmax*
    [0.01]. For each duplicate found, the output record begins with the
    single letter Y (exact match) or ~ (approximate match). If the two
    matching segments differ in length by more than a factor of 2 then
    we consider the duplicate to be either a subset (-) or a superset
    (+). Finally, we also note if two lines are the result of splitting
    a continuous line across the Dateline (|).
    For polygons we also consider the fractional difference in
    areas; duplicates must differ by less than *amax* [0.01]. By
    default, we compute the mean line separation. Use **+C**\ *cmin* to
    instead compute the median line separation and therefore a robust
    closeness value. Also by default we consider all distances between
    points on one line and another. Append **+p** to limit the
    comparison to points that project perpendicularly to points on the
    other line (and not its extension).

.. _-E:

**-E**\ **+p**\ \|\ **n** ]
    Reset the handedness of all polygons to match the given **+p**
    (counter-clockwise; positive) or **+n** (clockwise; negative). Implies **-Q+**.

.. _-F:

**-F**\ [**l**]
   Force input data to become polygons on output, i.e., close them explicitly if not
   already closed.  Optionally, append **l** to force line geometry.

.. _-I:

**-I**\ [**e**\ \|\ **i**]
    Determine the intersection locations between all pairs of polygons.
    Append **i** to only compute internal (i.e., self-intersecting
    polygons) crossovers or **e** to only compute external (i.e.,
    between paris of polygons) crossovers [Default is both].

.. _-N:

**-N**\ *pfile*\ [**+a**][\ **+p**\ *start*][**+r**][**+z**]
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

.. _-Q:

**-Q**\ [*unit*\ ][**+c**\ *min*\ [/*max*]][**+h**\ ][**+l**\ ][**+p**\ ][**+s**\ [**a**\ \|\ **d**]]
    Measure the area of all polygons or length of line segments. Use
    **-Q+h** to append the area to each polygons segment header [Default
    simply writes the area to stdout]. For polygons we also compute the
    centroid location while for line data we compute the mid-point
    (half-length) position. Append a distance unit to select the unit
    used (see :ref:`Unit_attributes`). Note that the area will depend on the current
    setting of :ref:`PROJ_ELLIPSOID <PROJ_ELLIPSOID>`; this should be a
    recent ellipsoid to get accurate results. The centroid is computed
    using the mean of the 3-D Cartesian vectors making up the polygon
    vertices, while the area is obtained via an equal-area projection.
    Normally, all input segments
    will be be reflected on output.  Use **c** to restrict processing to
    those whose length (or area for polygons) fall inside the specified
    range set by *min* and *max*.  If *max* is not set it defaults to infinity.
    To sort the segments based on their lengths or area, use **s** and
    append **a** for ascending and **d** for descending order [ascending]. 
    By default, we consider open polygons as lines.
    Append **+p** to close open polygons and thus consider all input
    as polygons, or append **+l** to consider all input as lines, even
    if closed.

.. _-R:

.. |Add_-Rgeo| replace:: Clips polygons to the map
    region, including map boundary to the polygon as needed. The result
    is a closed polygon.
.. include:: explain_-Rgeo.rst_

.. _-S:

**-S**\ **h**\ \|\ **i**\ \|\ **j**\ \|\ **s**\ \|\ **u**
    Spatial processing of polygons. Choose from **-Sh** which identifies
    perimeter and hole polygons (and flags/reverses them), **-Si** which returns
    the intersection of polygons (closed), **-Su** which returns the
    union of polygons (closed), **-Ss** which will split polygons that
    straddle the Dateline, and **-Sj** which will join polygons that
    were split by the Dateline.  Note: Only **-Ss** has been implemented.

.. _-T:

**-T**\ [*clippolygon*]
    Truncate polygons against the specified polygon given, possibly
    resulting in open polygons. If no argument is given to **-T** we
    create a clipping polygon from **-R** which then is required. Note
    that when the **-R** clipping is in effect we will also look for
    polygons of length 4 or 5 that exactly match the **-R** clipping polygon. 

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_distcalc.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_inside.rst_

.. include:: explain_precision.rst_

Example
-------

To turn all lines in the multisegment file lines.txt into closed polygons,
run

   ::

    gmt spatial lines.txt -F > polygons.txt

To compute the area of all geographic polygons in the multisegment file
polygons.txt, run

   ::

    gmt spatial polygons.txt -Q > areas.txt

Same data, but now orient all polygons to go counter-clockwise and write
their areas to the segment headers, run

   ::

    gmt spatial polygons.txt -Q+h -E+p > areas.txt

To determine the areas of all the polygon segments in the file janmayen_land_full.txt,
add this information to the segment headers, sort the segments from largest
to smallest in area but only keep polygons with area larger than 1000 sq. meters, run

   ::

    gmt spatial -Qe+h+p+c1000+sd -V janmayen_land_full.txt > largest_pols.txt

To determine the intersections between the polygons A.txt and B.txt, run

   ::

    gmt spatial A.txt B.txt -Ie > crossovers.txt

To truncate polygons A.txt against polygon B.txt, resulting in an open line segment, run

   ::

    gmt spatial A.txt -TB.txt > line.txt

Notes
-----

OGR/GMT files are considered complete datasets and thus you cannot specify more than one
at a given time. This causes problems if you want to examine the intersections of
two OGR/GMT files.  The solution is to convert them to regular datasets via
:doc:`gmtconvert` and then run **gmt spatial** on the converted files.

See Also
--------

:doc:`gmt`,
:doc:`gmtconvert`,
:doc:`gmtselect`,
:doc:`gmtsimplify`
