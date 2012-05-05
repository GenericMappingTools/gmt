**********
mapproject
**********


mapproject - Do forward and inverse map transformations, datum
conversions and geodesy

`Synopsis <#toc1>`_
-------------------

**mapproject** [ *table* ] **-J**\ *parameters*
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [
**-Ab**\ \|\ **B**\ \|\ **f**\ \|\ **F**\ \|\ **o**\ \|\ **O**\ [*lon0*/*lat0*]
] [ **-C**\ [*dx*/*dy*] ] [ **-Dc**\ \|\ **i**\ \|\ **p** ] [
**-E**\ [*datum*\ ] ] [ **-F**\ [*unit*\ ] ] [
**-G**\ [*x0*/*y0*][/\ *unit*][\ **+**\ \|\ **-**] ] [ **-I** ] [
**-L**\ *line.xy*\ [/*unit*][**+**\ ] ] [
**-N**\ [**a**\ \|\ **c**\ \|\ **g**\ \|\ **m**] ] [
**-Q**\ [**d**\ \|\ **e** ] [ **-S** ] [
**-T**\ [**h**\ ]\ *from*\ [/*to*] ] [ **-V**\ [*level*\ ] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-s**\ [*cols*\ ][\ **a**\ \|\ **r**]
] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**mapproject** reads (longitude, latitude) positions from *infiles* [or
standard input] and computes (x,y) coordinates using the specified map
projection and scales. Optionally, it can read (x,y) positions and
compute (longitude, latitude) values doing the inverse transformation.
This can be used to transform linear (x,y) points obtained by digitizing
a map of known projection to geographical coordinates. May also
calculate distances along track, to a fixed point, or closest approach
to a line. Finally, can be used to perform various datum conversions.
Additional data fields are permitted after the first 2 columns which
must have (longitude,latitude) or (x,y). See option **-:** on how to
read (latitude,longitude) files.

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
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest. Special case for the UTM projection:
    If **-C** is used and **-R** is not given then the region is set to
    coincide with the given UTM zone so as to preserve the full
    ellipsoidal solution (See RESTRICTIONS for more information).

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncol*\ ][**t**\ ]) data
    table file(s) holding a number of data columns. If no tables are
    given then we read from standard input.
**-Ab**\ \|\ **B**\ \|\ **f**\ \|\ **F**\ \|\ **o**\ \|\ **O**\ [*lon0*/*lat0*]
    **-Af** calculates the (forward) azimuth from fixed point *lon/lat*
    to each data point. Use **-Ab** to get back-azimuth from data points
    to fixed point. Use **-Ao** to get orientations (-90/90) rather than
    azimuths (0/360). Upper case **F**, **B** or **O** will convert from
    geodetic to geocentric latitudes and estimate azimuth of geodesics
    (assuming the current ellipsoid is not a sphere). If no fixed point
    is given then we compute the azimuth (or back-azimuth) from the
    previous point.
**-C**\ [*dx*/*dy*]
    Set center of projected coordinates to be at map projection center
    [Default is lower left corner]. Optionally, add offsets in the
    projected units to be added (or subtracted when **-I** is set) to
    (from) the projected coordinates, such as false eastings and
    northings for particular projection zones [0/0]. The unit used for
    the offsets is the plot distance unit in effect (see
    **PROJ\_LENGTH\_UNIT**) unless **-F** is used, in which case the
    offsets are in meters.
**-Dc**\ \|\ **i**\ \|\ **p**
    Temporarily override **PROJ\_LENGTH\_UNIT** and use **c** (cm),
    **i** (inch), or **p** (points) instead. Cannot be used with **-F**.
**-E**\ [*datum*\ ]
    Convert from geodetic (lon, lat, height) to Earth Centered Earth
    Fixed (ECEF) (x,y,z) coordinates (add **-I** for the inverse
    conversion). Append datum ID (see **-Qd**) or give
    *ellipsoid*:*dx*,\ *dy*,\ *dz* where *ellipsoid* may be an ellipsoid
    ID (see **-Qe**) or given as *a*\ [,*inv\_f*], where *a* is the
    semi-major axis and *inv\_f* is the inverse flattening (0 if
    omitted). If *datum* is - or not given we assume WGS-84.
**-F**\ [*unit*\ ]
    Force 1:1 scaling, i.e., output (or input, see **-I**) data are in
    actual projected meters. To specify other units, append the desired
    unit (see UNITS). Without **-F**, the output (or input, see **-I**)
    are in the units specified by **PROJ\_LENGTH\_UNIT** (but see
    **-D**).
**-G**\ [*x0*/*y0*][/\ *unit*][\ **+**\ \|\ **-**]
    Calculate distances along track *or* to the optional point set with
    **-G**\ *x0/y0*. Append the distance unit (see UNITS), including
    **c** (Cartesian distance using input coordinates) or **C**
    (Cartesian distance using projected coordinates). The **C** unit
    requires **-R** and **-J** to be set. With no fixed point is given
    we calculate cumulate distances along track. Append **-** to obtain
    incremental distance between successive points. Append **+** to
    specify the 2nd point via two extra columns in the input file.
**-I**
    Do the Inverse transformation, i.e. get (longitude,latitude) from
    (x,y) data.
**-L**\ *line.xy*\ [/*unit*][**+**\ ]
    Determine the shortest distance from the input data points to the
    line(s) given in the ASCII multisegment file *line.xy*. The distance
    and the coordinates of the nearest point will be appended to the
    output as three new columns. Append the distance unit (see UNITS),
    including **c** (Cartesian distance using input coordinates) or
    **C** (Cartesian distance using projected coordinates). The **C**
    unit requires **-R** and **-J** to be set. Finally, append **+** to
    report the line segment id and the fractional point number instead
    of lon/lat of the nearest point.
**-N**\ [**a**\ \|\ **c**\ \|\ **g**\ \|\ **m**]
    Convert from geodetic latitudes (using the current ellipsoid; see
    **PROJ\_ELLIPSOID**) to one of four different auxiliary latitudes
    (longitudes are unaffected). Choose from **a**\ uthalic,
    **c**\ onformal, **g**\ eocentric, and **m**\ eridional latitudes
    [geocentric]. Use **-I** to convert from auxiliary latitudes to
    geodetic latitudes.
**-Q**\ [**d**\ \|\ **e**
    List all projection parameters. To only list datums, use **-Qd**. To
    only list ellipsoids, use **-Qe**.
**-S**
    Suppress points that fall outside the region.
**-T**\ [**h**\ ]\ *from*\ [/*to*]
    Coordinate conversions between datums *from* and *to* using the
    standard Molodensky transformation. Use **-Th** if 3rd input column
    has height above ellipsoid [Default assumes height = 0, i.e., on the
    ellipsoid]. Specify datums using the datum ID (see **-Qd**) or give
    *ellipsoid*:*dx*,\ *dy*,\ *dz* where *ellipsoid* may be an ellipsoid
    ID (see **-Qe**) or given as *a*\ [,*inv\_f*], where *a* is the
    semi-major axis and *inv\_f* is the inverse flattening (0 if
    omitted). If *datum* is - or not given we assume WGS-84. **-T** may
    be used in conjunction with **-R** **-J** to change the datum before
    coordinate projection (add **-I** to apply the datum conversion
    after the inverse projection). Make sure that the
    **PROJ\_ELLIPSOID** setting is correct for your case.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-bi**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-bo**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary output. [Default is same as input].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ] (\*)
    Determine data gaps and line breaks.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*] (\*)
    Select input columns.
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns.
**-s**\ [*cols*\ ][\ **a**\ \|\ **r**] (\*)
    Set handling of NaN records.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Units <#toc6>`_
----------------

For map distance units, append *unit* **d** for arc degrees, **m** for
arc minutes, and **s** for arc seconds, or **e** for meters [Default],
**f** for feet, **k** for km, **M** for statute miles, and **n** for
nautical miles. By default we compute such distances using a spherical
approximation with great circles. Prepend **-** to a distance (or the
unit is no distance is given) to perform "Flat Earth" calculations
(quicker but less accurate) or prepend **+** to perform exact geodesic
calculations (slower but more accurate).

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

`Examples <#toc8>`_
-------------------

To transform a file with (longitude,latitude) into (x,y) positions in cm
on a Mercator grid for a given scale of 0.5 cm per degree, run

mapproject lonlatfile -R20/50/12/25 **-Jm**\ 0.5\ **c** > xyfile

To transform several 2-column, binary, double precision files with
(latitude,longitude) into (x,y) positions in inch on a Transverse
Mercator grid (central longitude 75W) for scale = 1:500000 and suppress
those points that would fall outside the map area, run

mapproject tracks.\* -R-80/-70/20/40 -Jt-75/1:500000 -: -S -Di -bo -bi2 > tmfile.b

To convert the geodetic coordinates (lon, lat, height) in the file
old.dat from the NAD27 CONUS datum (Datum ID 131 which uses the
Clarke-1866 ellipsoid) to WGS 84, run

mapproject old.dat -Th131 > new.dat

To compute the closest distance (in km) between each point in the input
file quakes.dat and the line segments given in the multisegment ASCII
file coastline.xy, run

mapproject quakes.dat -Lcoastline.xy/k > quake\_dist.dat

`Restrictions <#toc9>`_
-----------------------

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
from the central meridian, `and (3) <and.3.html>`_ same for Cassini
(**-JC**) but with a limit of only 4 degrees.

`Ellipsoids And Spheroids <#toc10>`_
------------------------------------

**GMT** will use ellipsoidal formulae if they are implemented and the
user have selected an ellipsoid as the reference shape (see
**PROJ\_ELLIPSOID** in **gmt.conf**). The user needs to be aware of a
few potential pitfalls: (1) For some projections, such as Transverse
Mercator, Albers, and Lambert’s conformal conic we use the ellipsoidal
expressions when the areas mapped are small, and switch to the spherical
expressions (and substituting the appropriate auxiliary latitudes) for
larger maps. The ellipsoidal formulae are used as follows: (a)
Transverse Mercator: When all points are within 10 degrees of central
meridian, (b) Conic projections when longitudinal range is less than 90
degrees, (c) Cassini projection when all points are within 4 degrees of
central meridian. (2) When you are trying to match some historical data
(e.g., coordinates obtained with a certain projection and a certain
reference ellipsoid) you may find that **GMT** gives results that are
slightly different. One likely source of this mismatch is that older
calculations often used less significant digits. For instance, Snyder’s
examples often use the Clarke 1866 ellipsoid (defined by him as having a
flattening f = 1/294.98). From f we get the eccentricity squared to be
0.00676862818 (this is what **GMT** uses), while Snyder rounds off and
uses 0.00676866. This difference can give discrepancies of several tens
of cm. If you need to reproduce coordinates projected with this slightly
different eccentricity, you should specify your own ellipsoid with the
same parameters as Clarke 1866, but with f = 1/294.97861076. Also, be
aware that older data may be referenced to different datums, and unless
you know which datum was used and convert all data to a common datum you
may experience mismatches of tens to hundreds of meters. (3) Finally, be
aware that **PROJ\_SCALE\_FACTOR** have certain default values for some
projections so you may have to override the setting in order to match
results produced with other settings.

`See Also <#toc11>`_
--------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*gmt.conf*\ (5) <gmt.conf.5.html>`_ ,
`*project*\ (1) <project.1.html>`_

`References <#toc12>`_
----------------------

Bomford, G., 1952, Geodesy, Oxford U. Press.
Snyder, J. P., 1987, Map Projections - A Working Manual, U.S.
Geological Survey Prof. Paper 1395.
Vanicek, P. and Krakiwsky, E, 1982, Geodesy - The Concepts,
North-Holland Publ., ISBN: 0 444 86149 1.

