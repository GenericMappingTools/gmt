.. index:: ! gmtspatial
.. include:: module_core_purpose.rst_

*******
spatial
*******

|gmtspatial_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt spatial** [ *table* ]
[ |-A|\ [**a**\ *min_dist*][*unit*]]
[ |-C| ]
[ |-D|\ [**+a**\ *amax*][**+c\|C**\ *cmax*][**+d**\ *dmax*][**+f**\ *file*][**+p**][**+s**\ *factor*] ]
[ |-E|\ **+p**\|\ **n** ]
[ |-F|\ [**l**] ]
[ |-I|\ [**e**\|\ **i**] ]
[ |-L|\ *dist*\ /*noise*\ /*offset* ]
[ |-N|\ *pfile*\ [**+a**][**+i**][**+p**\ [*start*]][**+r**][**+z**] ]
[ |-Q|\ [*unit*][**+c**\ *min*\ [/*max*]][**+h**][**+l**][**+p**][**+s**\ [**a**\|\ **d**]] ]
[ |SYN_OPT-R| ]
[ |-S|\ **b**\ *width*\|\ **h**\|\ **s** ]
[ |-T|\ [*clippolygon*] ]
[ |SYN_OPT-V| ]
[ |-W|\ *dist*\[*unit*][**+f**\|\ **l**] ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-j| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-q| ]
[ |SYN_OPT-s| ]
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

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

Optional Arguments
------------------

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
   ID to be included in the average.).  **Note**: The input data are assumed to
   contain (*lon, lat*) or (*x, y*), optionally followed by a *z* and a *weight* [1] column.
   We compute a weighted average of the location and *z* (if *weight* is present).

.. _-C:

**-C**
    Clips polygons to the map region, including map boundary to the
    polygon as needed. The result is a closed polygon (see |-T| for
    truncation instead). Requires |-R|.

.. _-D:

**-D**\ [**+c\|C**\ *cmax*][**+d**\ *dmax*][**+f**\ *file*][**+p**][**+s**\ *fact*]
    Check for duplicates among the input lines (or polygons).
    We consider both the cases of exact (same number and coordinates) and
    approximate matches (average distance between nearest points of two
    features is less than a threshold). We also consider that some features
    may have been reversed. By default, we compute the mean line separation.

    - **+c** - Set threshold of a pair's closeness (defined as the average distance
      between the features divided by their average length) [0.01].
    - **+C** - Use **+C**\ *cmin* to instead compute the median line separation
      and therefore a robust closeness value.
    - **+d** - Features are considered approximate matches if their
      minimum distance is less than *dmax* [0] (see `Units`_) and their
      closeness (**+c**) is less than *cmax*.
    - **+f** - Check if the input features already exist among the features in *file*. 
    - **+p** - Limit the comparison to points that project perpendicularly to points
      on the other line (and not its extension) [Default considers all distances
      between points on one line and another.

    For each duplicate found, the output record begins with the
    single letter **Y** (exact match) or **~** (approximate match). If the two
    matching segments differ in length by more than a factor of 2 then
    we consider the duplicate to be either a subset (**-**) or a superset
    (**+**) and are flagged accordingly. Finally, we also note if two lines
    are the result of splitting a continuous line across the Dateline (**|**).

.. _-E:

**-E**\ **+p**\|\ **n**
    Reset the handedness of all polygons to match the given **+p**
    (counter-clockwise; positive) or **+n** (clockwise; negative).

.. _-F:

**-F**\ [**l**]
   Force input data to become polygons on output, i.e., close them explicitly if not
   already closed.  Optionally, append **l** to force line geometry.

.. _-I:

**-I**\ [**e**\|\ **i**]
    Determine the intersection locations between all pairs of polygons.
    Append **i** to only compute internal (i.e., self-intersecting
    polygons) crossovers or **e** to only compute external (i.e.,
    between pairs of polygons) crossovers [Default is both].  Output
    records will list the coordinates of the crossing, the relative times
    along the two segments (i.e., floating point record numbers at the
    crossing), and the names of the two segments (as trailing text).

.. _-L:

**-L**\ *dist*\ /*noise*\ /*offset*
    Remove tile Lines.  These are superfluous lines that were digitized with a
    polygon but that all fall along the rectangular |-R| border and should be removed.
    Append *dist* (in m) [0], coordinate *noise* [1e-10], and max *offset* from gridlines [1e-10].

.. _-N:

**-N**\ *pfile*\ [**+a**][**+i**][**+p**\ [*start*]][**+r**][**+z**]
    Lines and polygons: Determine if one (or all) points of each feature in the
    input data are inside any of the polygons given in the *pfile*. If
    inside, then report which polygon it is. The polygon ID is taken from
    the aspatial value assigned to Z or the segment header (first |-Z|, then
    |-L| are scanned). By default the input segments that are found to be
    inside a polygon are written to standard output with the polygon ID
    encoded in the segment header as **-Z**\ *ID*. Modifiers can be used
    to adjust the process:

    - **+a** - All the points of a feature must be inside the polygon.
    - **+i** - Point clouds, determine the polygon ID for every individual input point
      and add it as the last output column.
    - **+p** - Instead of segment headers, assign a running ID number
      that is initialized to begin from *start* [0].
    - **+r** - Just report which polygon contains a feature.
    - **+z** - Add the IDs as an extra data column on output.

    Segments that fail to be inside a polygon are not written out. If
    more than one polygon contains the same segment we skip the second
    (and further) scenarios.

.. _-Q:

**-Q**\ [*unit*][**+c**\ *min*\ [/*max*]][**+h**][**+l**][**+p**][**+s**\ [**a**\|\ **d**]]
    Measure the area of all polygons or length of all line segments.
    For polygons we also compute the centroid location while for lines
    we compute the mid-point (half-length) position. For geographical
    data, optionally append a distance unit to select the unit used
    (see `Units`_) [k]. Note that the area will depend on the current
    setting of :term:`PROJ_ELLIPSOID`; this should be a recent ellipsoid
    to get accurate results. The centroid is computed using the mean of
    the 3-D Cartesian vectors making up the polygon vertices, while the
    area is obtained via a sum of areas for spherical triangles. Normally,
    all input segments will be be reflected on output. By default, we
    consider open polygons as lines and closed polygons as polygons.
    Use modifiers to change the above behavior:

    - **+c** - Restrict processing to those features whose length (or area
      for polygons) fall inside the specified range set by *min* and *max*.
      If *max* is not set it defaults to infinity.
    - **+h** - Append the area to each polygonʻs segment header [Default
      simply writes the area to standard output]. 
    - **+l** - Consider all input features as lines, even if closed.
    - **+p** - Close open polygons and thus consider all input as polygons.
    - **+s** - Sort the segments based on their lengths or area. Append **a**
      for ascending [Default] and **d** for descending order.

.. _-R:

.. |Add_-Rgeo| replace:: Clips polygons to the map
    region, including map boundary to the polygon as needed. The result
    is a closed polygon.
.. include:: explain_-Rgeo.rst_

.. _-S:
    **i** - Returns the intersection of input polygons (closed).
    **u** - Returns the union of input polygons (closed).
    **j** - Join polygons that were split by the Dateline.

**-S**\ **b**\ *width*\|\ **h**\|\ **s**
    Spatial processing of polygons. Choose from several directives:

    - **b** - Append *width* which computes a buffer polygon around lines.
    - **h** - Identifies perimeter and hole polygons (and flags/reverses them).
    - **s** - Split polygons that straddle the Dateline.

    **Note**: **-Sb** is a purely Cartesian operation so *width* must be in data units.
    That is, for geographical coordinates *width* must be provided in degrees or,
    preferably, project data into an equal-area projection with :doc:`mapproject`,
    compute the buffer and then convert back to geographical.

.. _-T:

**-T**\ [*clippolygon*]
    Truncate polygons and lines against the specified polygon given, possibly
    resulting in open polygons. If no argument is given to |-T| we
    create a clipping polygon from |-R| which then is required. Note
    that when the |-R| clipping is in effect we will also look for
    polygons of length 4 or 5 that exactly match the |-R| clipping polygon.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *dist*\[*unit*][**+f**\|\ **l**]
    Extend all segments with a new first and last point such that these points are *dist* away
    from their neighbor point in the direction implied by the two points at each end of the
    segment.  For geographic data you may append a *unit* (see `Units`_). To give separate
    distances for the two ends, give *distf*\[*unit*]/*distl*\[*unit*] instead.  Optionally,
    append either **+f** or **+l** to only extend the first or last point this way [both].
    The mode of geographical calculations depends on **-j**.

.. include:: explain_-aspatial.rst_

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

.. include:: explain_-q.rst_

.. include:: explain_-s.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_inside.rst_

.. include:: explain_precision.rst_

Examples
--------

To determine the centroid of the remote GSHHH high-resolution polygon for Australia,
as well as the land area in km squared, try::

    gmt spatial @GSHHS_h_Australia.txt -fg -Qk

To turn all lines in the multisegment file lines.txt into closed polygons,
run::

    gmt spatial lines.txt -F > polygons.txt

To append the polygon ID of every individual point in cloud.txt that is inside the
polygons in the file poly.txt and write that ID as the last column per output row, run::

    gmt spatial cloud.txt -Npoly.txt+i  > cloud_IDs.txt

To compute the area of all geographic polygons in the multisegment file
polygons.txt, run::

    gmt spatial polygons.txt -Q > areas.txt

Same data, but now orient all polygons to go counter-clockwise and write
their areas to the segment headers, run::

    gmt spatial polygons.txt -Q+h -E+p > areas.txt

To determine the areas of all the polygon segments in the file janmayen_land_full.txt,
add this information to the segment headers, sort the segments from largest
to smallest in area but only keep polygons with area larger than 1000 sq. meters, run::

    gmt spatial -Qe+h+p+c1000+sd -V janmayen_land_full.txt > largest_pols.txt

To determine the intersections between the polygons A.txt and B.txt, run::

    gmt spatial A.txt B.txt -Ie > crossovers.txt

To truncate polygons A.txt against polygon B.txt, resulting in an open line segment, run::

    gmt spatial A.txt -TB.txt > line.txt

If you want to plot a polygon with holes (donut polygon) from a multiple segment file
which contains both perimeters and holes, it could be necessary first to reorganize the file
so it can plotted with plot. To do this, run::

    gmt spatial file.txt -Sh > organized_file.txt

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
:doc:`gmtsimplify`,
:doc:`mapproject`
