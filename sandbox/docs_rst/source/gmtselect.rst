*********
gmtselect
*********

gmtselect - Select data table subsets based on multiple spatial criteria

`Synopsis <#toc1>`_
-------------------

**gmtselect** [ *table* ] [
**-A**\ *min\_area*\ [/*min\_level*/*max\_level*][\ **+r**\ \|\ **l**][\ **p**\ *percent*]
] [ **-C**\ *dist*\ [*unit*\ ]/\ *ptfile* ] [
**-D**\ *resolution*\ [**+**\ ] ] [ **-E**\ [**fn**\ ] ] [
**-F**\ *polygonfile* ] [ **-I**\ [**cflrsz**\ ] ] [
**-J**\ *parameters* ] [
**-L**\ [**p**\ ]\ *dist*\ [*unit*\ ]/\ *linefile* ] [
**-N**\ *maskvalues* ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-V**\ [*level*\ ]
] [ **-Z**\ *min/max*] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-s**\ [*cols*\ ][\ **a**\ \|\ **r**]
] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**gmtselect** is a filter that reads (longitude, latitude) positions
from the first 2 columns of *infiles* [or standard input] and uses a
combination of 1-6 criteria to pass or reject the records. Records can
be selected based on whether or not they are 1) inside a rectangular
region (**-R** [and **-J**]), 2) within *dist* km of any point in
*ptfile*, 3) within *dist* km of any line in *linefile*, 4) inside one
of the polygons in the *polygonfile*, 5) inside geographical features
(based on coastlines), or 6) has z-values within a given range. The
sense of the tests can be reversed for each of these 6 criteria by using
the **-I** option. See option **-:** on how to read (latitude,longitude)
files.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

None

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    data table file(s) holding a number of data columns. If no tables
    are given then we read from standard input.
**-A**\ *min\_area*\ [/*min\_level*/*max\_level*][\ **+r**\ \|\ **l**][\ **p**\ *percent*]
    Features with an area smaller than *min\_area* in km^2 or of
    hierarchical level that is lower than *min\_level* or higher than
    *max\_level* will not be plotted [Default is 0/0/4 (all features)].
    Level 2 (lakes) contains regular lakes and wide river bodies which
    we normally include as lakes; append **+r** to just get river-lakes
    or **+l** to just get regular lakes. Finally, append
    **+p**\ *percent* to exclude polygons whose percentage area of the
    corresponding full-resolution feature is less than *percent*. See
    GSHHS INFORMATION below for more details. Ignored unless **-N** is
    set.
**-C**\ *dist*\ [*unit*\ ]/\ *ptfile*
    Pass all records whose location is within *dist* of any of the
    points in the ASCII file *ptfile*. If *dist* is zero then the 3rd
    column of *ptfile* must have each point’s individual radius of
    influence. Distances are Cartesian and in user units; specify
    **-fg** to indicate spherical distances and append a distance unit
    (see UNITS). Alternatively, if **-R** and **-J** are used then
    geographic coordinates are projected to map coordinates (in cm,
    inch, or points, as determined by **PROJ\_LENGTH\_UNIT**) before
    Cartesian distances are compared to *dist*.
**-D**\ *resolution*\ [**+**\ ]
    Ignored unless **-N** is set. Selects the resolution of the
    coastline data set to use ((**f**)ull, (**h**)igh,
    (**i**)ntermediate, (**l**)ow, or (**c**)rude). The resolution drops
    off by ~80% between data sets. [Default is **l**]. Append )+) to
    automatically select a lower resolution should the one requested not
    be available [abort if not found]. Note that because the coastlines
    differ in details it is not guaranteed that a point will remain
    inside [or outside] when a different resolution is selected.
**-E**\ [**fn**\ ]
    Specify how points exactly on a polygon boundary should be
    considered. By default, such points are considered to be inside the
    polygon. Append **n** and/or **f** to change this behavior for the
    **-F** and **-N** options, respectively, so that boundary points are
    considered to be outside.
**-F**\ *polygonfile*
    Pass all records whose location is within one of the closed polygons
    in the multiple-segment file *polygonfile*. For spherical polygons
    (lon, lat), make sure no consecutive points are separated by 180
    degrees or more in longitude. Note that *polygonfile* must be in
    ASCII regardless of whether **-bi**\ [*ncols*\ ][*type*\ ] is used.
**-I**\ [**cflrsz**\ ]
    Reverses the sense of the test for each of the criteria specified:

    **c** select records NOT inside any point’s circle of influence.

    **f** select records NOT inside any of the polygons.

    **l** select records NOT within the specified distance of any line.

    **r** select records NOT inside the specified rectangular region.

    **s** select records NOT considered inside as specified by **-N**
    (and **-A**, **-D**).

    **z** select records NOT within the range specified by **-Z**.

**-J**\ *parameters* (\*)
    Select map projection.
**-L**\ [**p**\ ]\ *dist*\ [*unit*\ ]/\ *linefile*
    Pass all records whose location is within *dist* of any of the line
    segments in the ASCII multiple-segment file *linefile*. If *dist* is
    zero then we will scan each sub-header in the *ptfile* for an
    embedded **-D**\ *dist* setting that sets each line’s individual
    distance value. Distances are Cartesian and in user units; specify
    **-fg** to indicate spherical distances append a distance unit (see
    UNITS). Alternatively, if **-R** and **-J** are used then geographic
    coordinates are projected to map coordinates (in cm, inch, m, or
    points, as determined by **PROJ\_LENGTH\_UNIT**) before Cartesian
    distances are compared to *dist*. Use **-Lp** to ensure only points
    whose orthogonal projections onto the nearest line-segment fall
    within the segments endpoints [Default considers points "beyond" the
    line’s endpoints.
**-N**\ *maskvalues*
    Pass all records whose location is inside specified geographical
    features. Specify if records should be skipped (s) or kept (k) using
    1 of 2 formats:

    **-N**\ *wet/dry*.

    **-N**\ *ocean/land/lake/island/pond*.

    [Default is s/k/s/k/s (i.e., s/k), which passes all points on dry
    land].

**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest. If no map projection is supplied we
    implicitly set **-Jx**\ 1.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-Z**\ *min/max*]
    Pass all records whose 3rd column (z) lies within the given range.
    Input file must have at least three columns. To indicate no limit on
    min or max, specify a hyphen (-). If your 3rd column is absolute
    time then remember to supply **-f**\ 2T.
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
**-s**\ [*cols*\ ][\ **a**\ \|\ **r**] (\*)
    Set handling of NaN records.
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

This note applies to ASCII output only in combination with binary or
netCDF input or the **-:** option. See also the note below.

`Note On Processing Ascii Input Records <#toc8>`_
-------------------------------------------------

Unless you are using the **-:** option, selected ASCII input records are
copied verbatim to output. That means that options like **-foT** and
settings like **FORMAT\_FLOAT\_OUT** and **FORMAT\_GEO\_OUT** will not
have any effect on the output. On the other hand, it allows selecting
records with diverse content, including character strings, quoted or
not, comments, and other non-numerical content.

`Note On Distances <#toc9>`_
----------------------------

If options **-C** or **-L** are selected then distances are Cartesian
and in user units; use **-fg** to imply spherical distances in km and
geographical (lon, lat) coordinates. Alternatively, specify **-R** and
**-J** to measure projected Cartesian distances in map units (cm, inch,
or points, as determined by **PROJ\_LENGTH\_UNIT**).

This program has evolved over the years. Originally, the **-R** and
**-J** were mandatory in order to handle geographic data, but now there
is full support for spherical calculations. Thus, **-J** should only be
used if you want the tests to be applied on projected data and not the
original coordinates. If **-J** is used the distances given via **-C**
and **-L** are projected distances.

`Note On Segments <#toc10>`_
----------------------------

Segment headers in the input files are copied to output if one or more
records from a segment passes the test. Selection is always done point
by point, not by segment.

`Examples <#toc11>`_
--------------------

To extract the subset of data set that is within 300 km of any of the
points in pts.d but more than 100 km away from the lines in lines.d, run

gmtselect lonlatfile -fg -C300k/pts.d -L100/lines.d -Il > subset

Here, you must specify **-fg** so the program knows you are processing
geographical data.

To keep all points in data.d within the specified region, except the
points on land (as determined by the high-resolution coastlines), use

gmtselect data.d -R120/121/22/24 -Dh -Nk/s > subset

To return all points in quakes.d that are inside or on the spherical
polygon lonlatpath.d, try

gmtselect quakes.d -Flonlatpath.d -fg > subset1

To return all points in stations.d that are within 5 cm of the point in
origin.d for a certain projection, try

gmtselect stations.d -C5/origin.d -R20/50/-10/20 -JM20c
--PROJ\_LENGTH\_UNIT=cm > subset2

`Gshhs Information <#toc12>`_
-----------------------------

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

`See Also <#toc13>`_
--------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf(5)
,* <gmt.conf.html>`_\ `*grdlandmask*\ (1) <grdlandmask.html>`_ ,
`*pscoast*\ (1) <pscoast.html>`_
