.. index:: ! mapproject
.. include:: module_core_purpose.rst_

**********
mapproject
**********

|mapproject_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt mapproject** [ *table* ] |-J|\ *parameters*
|SYN_OPT-R|
[ |-A|\ **b**\|\ **f**\|\ **o**\ [*lon0*/*lat0*][**+v**] ]
[ |-C|\ [*dx*/*dy*][**+m**] ]
[ |-D|\ **c**\|\ **i**\|\ **p** ]
[ |-E|\ [*datum*] ]
[ |-F|\ [**e**\|\ **f**\|\ **k**\|\ **M**\|\ **n**\|\ **u**\|\ **c**\|\ **i**\|\ **p**] ]
[ |-G|\ [*lon0*/*lat0*][**+a**][**+i**][**+u**\ *unit*][**+v**] ]
[ |-I| ]
[ |-L|\ *table*\ [**+p**][**+u**\ *unit*] ]
[ |-N|\ [**a**\|\ **c**\|\ **g**\|\ **m**] ]
[ |-Q|\ [**d**\|\ **e**] ]
[ |-S| ]
[ |-T|\ [**h**]\ *from*\ [/*to*] ]
[ |SYN_OPT-V| ]
[ |-W|\ [**b**\|\ **B**\|\ **e**\|\ **E**\|\ **g**\|\ **h**\|\ **j**\|\ **m**\|\ **M**\|\ **n**\|\ **o**\|\ **O**\|\ **r**\|\ **R**\|\ **w**\|\ **s**\|\ **x**][**+n**\ [*nx*\ [/*ny*]]] ]
[ |-Z|\ [*speed*][**+a**][**+i**][**+f**][**+t**\ *epoch*] ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-j| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-q| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**mapproject** reads (*lon*, *lat*) positions from *tables* [or
standard input] and computes (*x, y*) coordinates using the specified map
projection and scales. Optionally, it can read (*x, y*) positions and
compute (*lon, lat*) values doing the inverse transformation.
This can be used to transform linear (*x, y*) points obtained by digitizing
a map of known projection to geographical coordinates. May also
calculate distances along track, to a fixed point, or closest approach
to a line.
Alternatively, can be used to perform various datum conversions.
Additional data fields are permitted after the first 2 columns which
must have (longitude,latitude) or (*x, y*). See option **-:** on how to
read (latitude,longitude) files.
Finally, **mapproject** can compute a variety of auxiliary output
data from input coordinates that make up a track.  Items like
azimuth, distances, distances to other lines, and travel-times
along lines can all be computed by using one or more of the options
|-A|, |-G|, |-L|, and |-Z|. **Note**: Depending on the Optional Arguments
listed below, most times **-J** or **-R** are not actually required.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

(Note that depending on the Optional Arguments listed below, sometimes -J and -R are not actually required.)

Optional Arguments
------------------

.. _-A:

**-Ab**\|\ **f**\|\ **o**\ [*lon0*/*lat0*][**+v**]
    Calculate azimuth along track *or* to the optional *fixed* point set
    with *lon0/lat0*.  Choose among several directives:

    - **b** - Calculate the back-azimuth from data points to the fixed point.
    - **f** - Calculate the forward azimuth from the fixed point to each data point.
    - **o** - Get orientations (-90/90) rather than azimuths (0/360).

    Use the **-je** option to compute azimuths on the ellipsoid instead of the sphere.
    **Note**: If no fixed point is given then we compute the azimuth (or back-azimuth) from the
    previous point. One modifier is available:

    - **+v** - Obtain a *variable* 2nd point (*lon0*/*lat0*) via columns 3-4
      in the input file.

    See `Output Order`_ for how |-A| affects the output record.  **Note**:
    If |-R| and |-J| are given the we project the coordinates first and
    then compute Cartesian angles instead.

.. _-C:

**-C**\ [*dx*/*dy*][**+m**]
    Set center of projected coordinates to be at map projection center
    [Default is lower left corner]. Optionally, add offsets in the
    projected units to be added (or subtracted when |-I| is set) to
    (from) the projected coordinates, such as false eastings and
    northings for particular projection zones [0/0]. The unit used for
    the offsets is the plot distance unit in effect (see
    :term:`PROJ_LENGTH_UNIT`) unless |-F| is used, in which case the
    offsets are in meters.  Alternatively, for the Mercator projection
    only, append **+m** to set the origin of the projected *y* coordinates
    to coincide with the standard parallel [Equator].

.. _-D:

**-Dc**\|\ **i**\|\ **p**
    Temporarily override :term:`PROJ_LENGTH_UNIT` and use **c** (cm),
    **i** (inch), or **p** (points) instead. Cannot be used with |-F|.

.. _-E:

**-E**\ [*datum*]
    Convert from geodetic (*lon, lat, height*) to Earth Centered Earth Fixed (ECEF) (*x, y, z*) coordinates
    (add |-I| for the inverse conversion). Append datum ID (see |-Q|\ **d**) or give
    *ellipsoid*:*dx*,\ *dy*,\ *dz* where *ellipsoid* may be an ellipsoid
    ID (see |-Q|\ **e**) or given as *a*\ [,\ *inv_f*], where *a* is the
    semi-major axis and *inv_f* is the inverse flattening (0 if
    omitted). If *datum* is - or not given we assume WGS-84.

.. _-F:

**-F**\ [**e**\|\ **f**\|\ **k**\|\ **M**\|\ **n**\|\ **u**\|\ **c**\|\ **i**\|\ **p**]
    Force 1:1 scaling, i.e., output (or input, see |-I|) data are in
    actual projected meters. To specify other units, append the desired
    unit (see `Units`_). Without |-F|, and when using the classic |-J| syntax, the output (or input, see |-I|)
    are in the units specified by :term:`PROJ_LENGTH_UNIT` (but see |-D|). This changes, however, to meters
    when PROJ4 syntax or EPSG is used in |-J| and that regardless of |-F| being used or not.

.. _-G:

**-G**\ [*lon0*/*lat0*][**+a**][**+i**][**+u**\ *unit*][**+v**]
    Calculate distances along track *or* to the optional *fixed* point set
    with |-G|\ *lon0*/*lat0*. If no fixed point is given
    we calculate *accumulated* distances whereas if a fixed point is given
    we calculate *incremental* distances.  You can modify this and other
    features via some modifiers:
    
    - **+a** - Select accumulated distances.
    - **+i** - Select incremental distances.
    - **+u** - Append the distance unit (see `Units`_ for available units and
      how distances are computed [great circle using authalic radius]),
      including **c** (Cartesian distance using input coordinates) or **C**
      (Cartesian distance using projected coordinates). The **C** unit
      requires |-R| and |-J| to be set and all output coordinates will be
      reported as projected.
    - **+v** - Obtain a *variable* 2nd point (*lon0*/*lat0*) via columns
      3-4 in the input file; this updates the fixed point per record and thus the
      selection defaults to incremental distances.
    
    **Notes**: (1) If both **+a** and **+i** are given we will report both
    types of distances. (2) See `Output Order`_ for how |-G| affects the output record.

.. _-I:

**-I**
    Do the Inverse transformation, i.e., get (longitude,latitude) from (*x, y*) data.

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-L:

**-L**\ *table*\ [**+p**][**+u**\ *unit* \|\ *c*  \|\ *C*]
    Determine the shortest distance from the input data points to the
    line(s) given in the ASCII multisegment file *table*. The distance
    and the coordinates of the nearest point will be appended to the
    output as three new columns. Consider these modifiers:

   - **+p** - Report the line segment id *seg* and the fractional point number
     *pnr* instead of *lon*/*lat* of the nearest point.
    - **+u** - Append the distance unit (see `Units`_ for available units and
      how distances are computed [great circle using authalic radius]),
      including **c** (Cartesian distance using input coordinates) or **C**
      (Cartesian distance using projected coordinates). The **C** unit
      requires |-R| and |-J| to be set and all output coordinates will be
      reported as projected.
    
    **Notes**: (1) Calculation mode for geographic data is spherical, hence **-je**
    cannot be used in combination with |-L|. (2) See `Output Order`_ for how |-L|
    affects the output record.

.. _-N:

**-N**\ [**a**\|\ **c**\|\ **g**\|\ **m**]
    Convert from geodetic latitudes (using the current ellipsoid; see
    :term:`PROJ_ELLIPSOID`) to one of four different auxiliary latitudes
    (longitudes are unaffected). Choose from these directives:

    - **a** - Convert to authalic latitudes.
    - **c** - Convert to conformal latitudes.
    - **g** - Convert to geocentric latitudes [Default].
    - **m** - Convert to meridional latitudes.
    
    Use |-I| to instead convert from auxiliary latitudes to geodetic latitudes.

.. _-Q:

**-Q**\ [**d**\|\ **e**]
    List all projection parameters. To only list datums, use |-Q|\d. To
    only list ellipsoids, use |-Q|\e.

.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-S:

**-S**
    Suppress points that fall outside the region.

.. _-T:

**-T**\ [**h**]\ *from*\ [/*to*]
    Coordinate conversions between datums *from* and *to* using the
    standard Molodensky transformation. Use |-T|\ **h** if 3rd input column
    has height above ellipsoid [Default assumes height = 0, i.e., on the
    ellipsoid]. Specify datums using the datum ID (see |-Q|\ **d**) or give
    *ellipsoid*:*dx*,\ *dy*,\ *dz* where *ellipsoid* may be an ellipsoid
    ID (see |-Q|\ **e**) or given as *a*\ [,\ *inv_f*], where *a* is the
    semi-major axis and *inv_f* is the inverse flattening (0 if
    omitted). If *datum* is - or not given we assume WGS-84. |-T| may
    be used in conjunction with |-R| |-J| to change the datum before
    coordinate projection (add |-I| to apply the datum conversion
    after the inverse projection). Make sure that the
    :term:`PROJ_ELLIPSOID` setting is correct for your case.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [**b**\|\ **B**\|\ **e**\|\ **E**\|\ **g**\|\ **h**\|\ **j**\|\ **m**\|\ **M**\|\ **n**\|\ **o**\|\ **O**\|\ **r**\|\ **R**\|\ **s**\|\ **w**\|\ **x**][**+n**\ [*nx*\ [/*ny*]]]
    Report a variety of plot dimensions or map regions in projected or geographic units.
    No input files are read. With no argument we report the map width and height.
    The chosen unit of reported plot dimensions may be changed via |-D|. For
    other results, select from these directives:

    - **b** - Get the bounding box in longitude and latitude.
    - **B** - Same, but get the result in -Rw/e/s/n string format returned as trailing text.
    - **e** - Return the coordinates of the rectangular area encompassing the non-rectangular
      area defined by your |-R| |-J|.
    - **E** - Same, but in -Rw/e/s/n string format returned as trailing text.
    - **g** - Output the plot coordinates of the appended map point *lon*/*lat*.
    - **h** - Only output the height of the map.
    - **j** - Output the map coordinates of a reference point by appending its *code* (with
      standard two-character justification codes).
    - **n** - Same, but appended reference point *rx*/*ry* is given as normalized positions
      in the 0-1 range.
    - **o** - If an oblique domain is set via |-R|\ *xmin/xmax/ymin/ymax*\ **+u**\ *unit* then
      return the diagonal corner coordinates in degrees (in the order *llx urx lly ury*).
    - **O** - Same, but get the equivalent |-R| string format returned as trailing text.
    - **m** - Get the rectangular region in projected plot coordinates instead.
    - **M** - Same, but returned in |-R| string format returned as trailing text.
    - **r** - Output the rectangular domain that covers an oblique area as defined by |-R| |-J|.
    - **R** - Same, but get the result in |-R| string format returned as trailing text.
    - **s** - Output the map scale (i_scale) in the form 1:xxxxxx.
    - **w** - Only output the width of the map in current plot units.
    - **x** - Output the map coordinates of the specific plot reference point *px*/*py*.

    Optionally (for **e** or **r**), append modifier **+n** to set how many points [100]
    you want along each side for a closed polygon of the oblique area instead.

.. figure:: /_images/GMT_obl_regions.*
   :width: 600 px
   :align: center

   Comparing oblique (red outline) and regular (just meridians and parallels; black outline) regions.
   (left) Some domains are oblique (their perimeters are not following meridians and parallels).
   We can use |-W|\ **r**\ \|\ **R** to obtain the enclosing meridian/parallel box or the |-R| string
   for that region. (right) Other domains are not oblique but their enclosing rectangular box in
   the map projection will be.  We can explore |-W|\ **e**\ \|\ **E** to obtain the geographic coordinates
   of the encompassing oblique rectangle or the |-R| string for that region.

.. _-Z:

**-Z**\ [*speed*][**+a**][**+i**][**+f**][**+t**\ *epoch*]
    Calculate travel times along track as specified with |-G|.
    Append a constant speed unit; if missing we expect to read
    a variable speed from column 3.  The speed is expected to be
    in the distance units set via |-G| per time unit controlled
    by :term:`TIME_UNIT` [m/s].  A few modifiers are available:

    - **+i** - Output *incremental* travel times between successive points.
    - **+a** - Output *accumulated* travel times.
    - **+f** - Format the accumulated (elapsed) travel time according
      to the ISO 8601 convention. As for the number of decimals used to
      represent seconds we consult the :term:`FORMAT_CLOCK_OUT` setting.
    - **+t** - Append *epoch* to report absolute times (ETA) for successive points.
      Because of the need for incremental distances the |-G| option with the
      **+i** modifier is required.

    **Note**: See `Output Order`_ for how |-Z| affects the output record.

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

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-q.rst_

.. include:: explain_-s.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_


Examples
--------

.. include:: explain_example.rst_

To transform a remote file with (latitude,longitude) into (*x, y*) positions in cm
on a Mercator grid for a given scale of 0.5 cm per degree and selected region, run::

  gmt mapproject @waypoints.txt -R-180/180/-72/72 -Jm0.5c -: > xyfile

To convert UTM coordinates in meters to geographic locations, given
a file utm.txt and knowing the UTM zone (and zone or hemisphere), try::

  gmt mapproject utm.txt -Ju+11/1:1 -C -I -F


To transform several 2-column, binary, double precision files with
(latitude,longitude) into (*x, y*) positions in inch on a Transverse
Mercator grid (central longitude 75W) for scale = 1:500000 and suppress
those points that would fall outside the map area, run::

  gmt mapproject tracks.* -R-80/-70/20/40 -Jt-75/1:500000 -: -S -Di -bo -bi2 > tmfile.b

To convert the geodetic coordinates (*lon, lat, height*) in the file
old.txt from the NAD27 CONUS datum (Datum ID 131 which uses the
Clarke-1866 ellipsoid) to WGS 84, run::

  gmt mapproject old.txt -Th131 > new.txt

To compute the closest distance (in km) between each point in the input
file quakes.txt and the line segments given in the multisegment ASCII
file coastline.txt, run::

  gmt mapproject quakes.txt -Lcoastline.txt+uk > quake_dist.txt

Given a file pos.txt with use Cartesian coordinates (say in meters or miles), compute
accumulated distance along track with::

  gmt mapproject pos.txt -G+uc > cum_distances.txt

Given a file with longitude and latitude, compute both incremental
and accumulated distance along track, and estimate travel times
assuming a fixed speed of 12 knots.  We do this with::

  gmt mapproject track.txt -G+un+a+i -Z12+a --TIME_UNIT=h > elapsed_time.txt

where :term:`TIME_UNIT` is set to hour so that the speed is
measured in nm (set by |-G|) per hour (set by :term:`TIME_UNIT`).
Elapsed times will be reported in hours (unless **+f** is added to |-Z|
for ISO elapsed time).

To determine the geographic coordinates of the mid-point of this transverse Mercator map, try::

  gmt mapproject -R-80/-70/20/40 -Jt-75/1:500000 -WjCM > mid_point.txt

To determine the rectangular region that encompasses the oblique region
defined by an oblique Mercator projection, try::

  gmt mapproject -R270/20/305/25+r -JOc280/25.5/22/69/2c -WR

To determine the oblique region string (in degrees) that corresponds to a rectangular
(but oblique) region specified in projected units defined by an oblique Mercator projection, try::

  gmt mapproject -R-2800/2400/-570/630+uk -Joc190/25/266/68/1:1 -WO

To instead get a closed polygon of the oblique area in geographical coordinates, try::

  gmt mapproject -R-2800/2400/-570/630+uk -Joc190/25/266/68/1:1 -Wr+n > polygon.txt

To find the region string that corresponds to the rectangular region that encompasses
the projected region defined by a stereographic projection, try::

  gmt mapproject -JS36/90/30c -R-15/60/68/90 -WE

To obtain the azimuth of a railroad using the points where it enters and leaves a city, try::

  echo -87.7447873 42.1192976 -87.7725841 42.1523955 | gmt mapproject -AF+v -fg -o4

Centering Output Region
-----------------------

The rectangular input region set with |-R| will in general be mapped
into a non-rectangular grid. Unless |-C| is set, the leftmost point on
this grid has xvalue = 0.0, and the lowermost point will have yvalue =
0.0. Thus, before you digitize a map, run the extreme map coordinates
through **mapproject** using the appropriate scale and see what (*x, y*)
values they are mapped onto. Use these values when setting up for
digitizing in order to have the inverse transformation work correctly,
or alternatively, use **gmt math** to scale and shift the (*x, y*) values
before transforming.

.. include:: explain_ellipsoidal.rst_

Output Order
------------

The production order for the geodetic and temporal columns produced by the
options |-A|, |-G|, |-L|, and |-Z| is fixed and follows the
alphabetical order of the options.  Hence, the order in which these options
appear on the command line is irrelevant.  The actual output order
can of course be modulated further via **-o**.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtvector`,
:doc:`project`

References
----------

Bomford, G., 1952, Geodesy, Oxford U. Press.

Snyder, J. P., 1987, Map Projections - A Working Manual, U.S. Geological
Survey Prof. Paper 1395.

Vanicek, P. and Krakiwsky, E, 1982, Geodesy - The Concepts,
North-Holland Publ., ISBN: 0 444 86149 1.
