.. index:: ! mapproject
.. include:: module_core_purpose.rst_

**********
mapproject
**********

|mapproject_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt mapproject** [ *tables* ] |-J|\ *parameters*
|SYN_OPT-R|
[ |-A|\ **b**\|\ **B**\|\ **f**\|\ **F**\|\ **o**\|\ **O**\ [*lon0*/*lat0*][**+v**] ]
[ |-C|\ [*dx*/*dy*][**+m**] ]
[ |-D|\ **c**\|\ **i**\|\ **p** ]
[ |-E|\ [*datum*] ] [ |-F|\ [*unit*] ]
[ |-G|\ [*lon0*/*lat0*][**+a**][**+i**][**+u**\ *unit*][**+v**] ]
[ |-I| ]
[ |-L|\ *line.xy*\ [**+u**\ *unit*][**+p**] ]
[ |-N|\ [**a**\|\ **c**\|\ **g**\|\ **m**] ]
[ |-Q|\ [**d**\|\ **e**] ]
[ |-S| ]
[ |-T|\ [**h**]\ *from*\ [/*to*] ]
[ |SYN_OPT-V| ]
[ |-W|\ [**g**\|\ **h**\|\ **j**\|\ **n**\|\ **w**\|\ **x**] ]
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

**mapproject** reads (longitude, latitude) positions from *tables* [or
standard input] and computes (x,y) coordinates using the specified map
projection and scales. Optionally, it can read (x,y) positions and
compute (longitude, latitude) values doing the inverse transformation.
This can be used to transform linear (x,y) points obtained by digitizing
a map of known projection to geographical coordinates. May also
calculate distances along track, to a fixed point, or closest approach
to a line.
Alternatively, can be used to perform various datum conversions.
Additional data fields are permitted after the first 2 columns which
must have (longitude,latitude) or (x,y). See option **-:** on how to
read (latitude,longitude) files.
Finally, **mapproject** can compute a variety of auxiliary output
data from input coordinates that make up a track.  Items like
azimuth, distances, distances to other lines, and travel-times
along lines can all be computed by using one or more of the options
**-A**, **-G**, **-L**, and **-Z**.

Required Arguments
------------------

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. _-R:

.. |Add_-R| replace:: Special case for the UTM
    projection: If **-C** is used and **-R** is not given then the
    region is set to coincide with the given UTM zone so as to preserve
    the full ellipsoidal solution (See RESTRICTIONS for more information).
.. include:: explain_-R.rst_

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-A:

**-Ab**\|\ **B**\|\ **f**\|\ **F**\|\ **o**\|\ **O**\ [*lon0*/*lat0*][**+v**]
    Calculate azimuth along track *or* to the optional *fixed* point set
    with *lon0/lat0*.  **-Af** calculates the (forward) azimuth
    to each data point. Use **-Ab** to get back-azimuth from data points
    to fixed point. Use **-Ao** to get orientations (-90/90) rather than
    azimuths (0/360). Upper case **F**, **B** or **O** will convert from
    geodetic to geocentric latitudes and estimate azimuth of geodesics
    (assuming the current ellipsoid is not a sphere). If no fixed point
    is given then we compute the azimuth (or back-azimuth) from the
    previous point.  Alternatively, append **+v** to obtain a
    *variable* 2nd point (*lon0*/*lat0*) via columns 3-4 in the input file.
    See `Output Order`_ for how **-A** affects the output record.

.. _-C:

**-C**\ [*dx*/*dy*][**+m**]
    Set center of projected coordinates to be at map projection center
    [Default is lower left corner]. Optionally, add offsets in the
    projected units to be added (or subtracted when **-I** is set) to
    (from) the projected coordinates, such as false eastings and
    northings for particular projection zones [0/0]. The unit used for
    the offsets is the plot distance unit in effect (see
    :term:`PROJ_LENGTH_UNIT`) unless **-F** is used, in which case the
    offsets are in meters.  Alternatively, for the Mercator projection
    only, append **+m** to set the origin of the projected *y* coordinates
    to coincide with the standard parallel [Equator].

.. _-D:

**-Dc**\|\ **i**\|\ **p**
    Temporarily override :term:`PROJ_LENGTH_UNIT` and use **c** (cm),
    **i** (inch), or **p** (points) instead. Cannot be used with **-F**.

.. _-E:

**-E**\ [*datum*]
    Convert from geodetic (lon, lat, height) to Earth Centered Earth Fixed (ECEF) (x,y,z) coordinates
    (add **-I** for the inverse conversion). Append datum ID (see **-Qd**) or give
    *ellipsoid*:*dx*,\ *dy*,\ *dz* where *ellipsoid* may be an ellipsoid
    ID (see **-Qe**) or given as *a*\ [,\ *inv_f*], where *a* is the
    semi-major axis and *inv_f* is the inverse flattening (0 if
    omitted). If *datum* is - or not given we assume WGS-84.

.. _-F:

**-F**\ [*unit*]
    Force 1:1 scaling, i.e., output (or input, see **-I**) data are in
    actual projected meters. To specify other units, append the desired
    unit (see `Units`_). Without **-F**, the output (or input, see **-I**)
    are in the units specified by :term:`PROJ_LENGTH_UNIT` (but see
    **-D**).

.. _-G:

**-G**\ [*lon0*/*lat0*][**+a**][**+i**][**+u**\ *unit*][**+v**]
    Calculate distances along track *or* to the optional *fixed* point set
    with **-G**\ *lon0*/*lat0*. Append the distance unit with **+u** (see `Units`_
    for available units and how distances are computed [great circle using authalic
    radius]), including **c** (Cartesian distance using input coordinates) or **C**
    (Cartesian distance using projected coordinates). The **C** unit
    requires **-R** and **-J** to be set. If no fixed point is given
    we calculate *accumulated* distances whereas if a fixed point is given
    we calculate *incremental* distances.  You can override these defaults
    by adding **+a** for accumulated or **+i** for incremental distances.
    If both **+a** and **+i** are given we will report both types of distances.
    Append **+v** to obtain a *variable* 2nd point (*lon0*/*lat0*) via columns
    3-4 in the input file; this updates the fixed point per record and thus the
    selection defaults to incremental distances.
    See `Output Order`_ for how **-G** affects the output record.

.. _-I:

**-I**
    Do the Inverse transformation, i.e., get (longitude,latitude) from (x,y) data.

.. _-L:

**-L**\ *line.xy*\ [**+u**\ *unit* \|\ *c*  \|\ *C*][**+p**]
    Determine the shortest distance from the input data points to the
    line(s) given in the ASCII multisegment file *line.xy*. The distance
    and the coordinates of the nearest point will be appended to the
    output as three new columns. Append the distance unit via **+u** (see `Units`_
    for available units and how distances are computed [great circle using authalic radius]),
    including **c** (Cartesian distance using input coordinates) or
    **C** (Cartesian distance using projected coordinates). Note that these **c** and **C** are
    not listed in  `Units`_ and would be used for example as **+uc**. The **C**
    unit requires **-R** and **-J** to be set. Finally, append **+p** to
    report the line segment id and the fractional point number instead
    of lon/lat of the nearest point.
    See `Output Order`_ for how **-L** affects the output record.

.. _-N:

**-N**\ [**a**\|\ **c**\|\ **g**\|\ **m**]
    Convert from geodetic latitudes (using the current ellipsoid; see
    :term:`PROJ_ELLIPSOID`) to one of four different auxiliary latitudes
    (longitudes are unaffected). Choose from **a**\ uthalic,
    **c**\ onformal, **g**\ eocentric, and **m**\ eridional latitudes
    [geocentric]. Use **-I** to convert from auxiliary latitudes to
    geodetic latitudes.

.. _-Q:

**-Q**\ [**d**\|\ **e**]
    List all projection parameters. To only list datums, use **-Qd**. To
    only list ellipsoids, use **-Qe**.

.. _-S:

**-S**
    Suppress points that fall outside the region.

.. _-T:

**-T**\ [**h**]\ *from*\ [/*to*]
    Coordinate conversions between datums *from* and *to* using the
    standard Molodensky transformation. Use **-Th** if 3rd input column
    has height above ellipsoid [Default assumes height = 0, i.e., on the
    ellipsoid]. Specify datums using the datum ID (see **-Qd**) or give
    *ellipsoid*:*dx*,\ *dy*,\ *dz* where *ellipsoid* may be an ellipsoid
    ID (see **-Qe**) or given as *a*\ [,\ *inv_f*], where *a* is the
    semi-major axis and *inv_f* is the inverse flattening (0 if
    omitted). If *datum* is - or not given we assume WGS-84. **-T** may
    be used in conjunction with **-R** **-J** to change the datum before
    coordinate projection (add **-I** to apply the datum conversion
    after the inverse projection). Make sure that the
    :term:`PROJ_ELLIPSOID` setting is correct for your case.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [**g**\|\ **h**\|\ **j**\|\ **n**\|\ **w**\|\ **x**]
    Prints map width and height on standard output.  No input files are read.
    To only output the width or the height, append **w** or **h**, respectively.
    To output the plot coordinates of a map point, give **g**\ *lon*/*lat*.
    The units of reported plot dimensions may be changed via **-D**.
    To output the map coordinates of a reference point, select **j**\ *code* (with
    standard two-character justification codes), **n**\ *rx*/*ry*, where the reference
    point is given as normalized positions in the 0-1 range, or **x**\ *px*/*py*,
    where a plot point is given directly [Default returns the width and height of the map].

.. _-Z:

**-Z**\ [*speed*][**+a**][**+i**][**+f**][**+t**\ *epoch*]
    Calculate travel times along track as specified with **-G**.
    Append a constant speed unit; if missing we expect to read
    a variable speed from column 3.  The speed is expected to be
    in the distance units set via **-G** per time unit controlled
    by :term:`TIME_UNIT` [m/s].  Append **+i** to output
    *incremental* travel times between successive points, **+a**
    to obtain *accumulated* travel times, or both to get both kinds
    of time information.  Use **+f** to format the accumulated
    (elapsed) travel time according to the ISO 8601 convention.
    As for the number of decimals used to represent seconds we
    consult the :term:`FORMAT_CLOCK_OUT`
    setting. Finally, append **+t**\ *epoch* to report absolute
    times (ETA) for successive points. Finally, because of the
    need for incremental distances the **-G** option with the
    **+i** modifier is required.
    See `Output Order`_ for how **-Z** affects the output record.

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

To transform a remote file with (latitude,longitude) into (x,y) positions in cm
on a Mercator grid for a given scale of 0.5 cm per degree and selected region, run

   ::

    gmt mapproject @waypoints.txt -R-180/180/-72/72 -Jm0.5c -: > xyfile

To convert UTM coordinates in meters to geographic locations, given
a file utm.txt and knowing the UTM zone (and zone or hemisphere), try

   ::

    gmt mapproject utm.txt -Ju+11/1:1 -C -I -F


To transform several 2-column, binary, double precision files with
(latitude,longitude) into (x,y) positions in inch on a Transverse
Mercator grid (central longitude 75W) for scale = 1:500000 and suppress
those points that would fall outside the map area, run

   ::

    gmt mapproject tracks.* -R-80/-70/20/40 -Jt-75/1:500000 -: -S -Di -bo -bi2 > tmfile.b

To convert the geodetic coordinates (lon, lat, height) in the file
old.txt from the NAD27 CONUS datum (Datum ID 131 which uses the
Clarke-1866 ellipsoid) to WGS 84, run

   ::

    gmt mapproject old.txt -Th131 > new.txt

To compute the closest distance (in km) between each point in the input
file quakes.txt and the line segments given in the multisegment ASCII
file coastline.xy, run

   ::

    gmt mapproject quakes.txt -Lcoastline.xy+uk > quake_dist.txt

Given a file with longitude and latitude, compute both incremental
and accumulated distance along track, and estimate travel times
assuming a fixed speed of 12 knots.  We do this with

   ::

    gmt mapproject track.txt -G+un+a+i -Z12+a --TIME_UNIT=h > elapsed_time.txt

where :term:`TIME_UNIT` is set to hour so that the speed is
measured in nm (set by **-G**) per hour (set by :term:`TIME_UNIT`).
Elapsed times will be reported in hours (unless **+f** is added to **-Z**
for ISO elapsed time).

To determine the geographic coordinates of the mid-point of this transverse Mercator map, try

   ::

    gmt mapproject -R-80/-70/20/40 -Jt-75/1:500000 -WjCM > mid_point.txt

where :term:`TIME_UNIT` is set to hour so that the speed is

Restrictions
------------

The rectangular input region set with **-R** will in general be mapped
into a non-rectangular grid. Unless **-C** is set, the leftmost point on
this grid has xvalue = 0.0, and the lowermost point will have yvalue =
0.0. Thus, before you digitize a map, run the extreme map coordinates
through **mapproject** using the appropriate scale and see what (x,y)
values they are mapped onto. Use these values when setting up for
digitizing in order to have the inverse transformation work correctly,
or alternatively, use **awk** to scale and shift the (x,y) values before
transforming.

For some projection, a spherical solution may be used despite the user
having selected an ellipsoid. This occurs when the users **-R** setting
implies a region that exceeds the domain in which the ellipsoidal series
expansions are valid. These are the conditions: (1) Lambert Conformal
Conic (**-JL**)and Albers Equal-Area (**-JB**) will use the spherical
solution when the map scale exceeds 1.0E7. (2) Transverse Mercator
(**-JT**) and UTM (**-JU**) will will use the spherical solution when
either the west or east boundary given in **-R** is more than 10 degrees
from the central meridian, and (3) same for Cassini
(**-JC**) but with a limit of only 4 degrees.

Ellipsoids And Spheroids
------------------------

GMT will use ellipsoidal formulae if they are implemented and the
user have selected an ellipsoid as the reference shape (see
:term:`PROJ_ELLIPSOID`). The user needs to be aware of a
few potential pitfalls: (1) For some projections, such as Transverse
Mercator, Albers, and Lambert's conformal conic we use the ellipsoidal
expressions when the areas mapped are small, and switch to the spherical
expressions (and substituting the appropriate auxiliary latitudes) for
larger maps. The ellipsoidal formulae are used as follows: (a)
Transverse Mercator: When all points are within 10 degrees of central
meridian, (b) Conic projections when longitudinal range is less than 90
degrees, (c) Cassini projection when all points are within 4 degrees of
central meridian. (2) When you are trying to match some historical data
(e.g., coordinates obtained with a certain projection and a certain
reference ellipsoid) you may find that GMT gives results that are
slightly different. One likely source of this mismatch is that older
calculations often used less significant digits. For instance, Snyder's
examples often use the Clarke 1866 ellipsoid (defined by him as having a
flattening f = 1/294.98). From f we get the eccentricity squared to be
0.00676862818 (this is what GMT uses), while Snyder rounds off and
uses 0.00676866. This difference can give discrepancies of several tens
of cm. If you need to reproduce coordinates projected with this slightly
different eccentricity, you should specify your own ellipsoid with the
same parameters as Clarke 1866, but with f = 1/294.97861076. Also, be
aware that older data may be referenced to different datums, and unless
you know which datum was used and convert all data to a common datum you
may experience mismatches of tens to hundreds of meters. (3) Finally, be
aware that :term:`PROJ_SCALE_FACTOR` have certain default values for some
projections so you may have to override the setting in order to match
results produced with other settings.

Output Order
------------

The production order for the geodetic and temporal columns produced by the
options **-A**, **-G**, **-L**, and **-Z** is fixed and follows the
alphabetical order of the options.  Hence, the order these options
appear on the command line is irrelevant.  The actual output order
can of course be modulated via **-o**.

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
