***********
grdlandmask
***********

grdlandmask - Create a "wet-dry" mask grid from shoreline data base

`Synopsis <#toc1>`_
-------------------

**grdlandmask** **-G**\ *mask\_grd\_file*]
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [
**-A**\ *min\_area*\ [/*min\_level*/*max\_level*][\ **+r**\ \|\ **l**][\ **p**\ *percent*]
] [ **-D**\ *resolution*\ [**+**\ ] ] [ **-N**\ *maskvalues*\ [**o**\ ]
] [ **-V**\ [*level*\ ] ] [ **-r** ]

`Description <#toc2>`_
----------------------

**grdlandmask** reads the selected shoreline database and uses that
information to decide which nodes in the specified grid are over land or
over water. The nodes defined by the selected region and lattice spacing
will be set according to one of two criteria: (1) land vs water, `or
(2) <or.2.html>`_ the more detailed (hierarchical) ocean vs land vs lake
vs island vs pond. The resulting mask may be used in subsequent
operations involving **grdmath** to mask out data from land [or water]
areas.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-G**\ *mask\_grd\_file*]
    Name of resulting output mask grid file. (See GRID FILE FORMATS
    below).
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
    *x\_inc* [and optionally *y\_inc*] is the grid spacing. Optionally,
    append a suffix modifier. **Geographical (degrees) coordinates**:
    Append **m** to indicate arc minutes or **s** to indicate arc
    seconds. If one of the units **e**, **f**, **k**, **M**, or **n** is
    appended instead, the increment is assumed to be given in meter,
    feet, km, Miles, or nautical miles, respectively, and will be
    converted to the equivalent degrees longitude at the middle latitude
    of the region (the conversion depends on **PROJ\_ELLIPSOID**). If
    /*y\_inc* is given but set to 0 it will be reset equal to *x\_inc*;
    otherwise it will be converted to degrees latitude. **All
    coordinates**: If **=** is appended then the corresponding max *x*
    (*east*) or *y* (*north*) may be slightly adjusted to fit exactly
    the given increment [by default the increment may be adjusted
    slightly to fit the given domain]. Finally, instead of giving an
    increment you may specify the *number of nodes* desired by appending
    **+** to the supplied integer argument; the increment is then
    recalculated from the number of nodes and the domain. The resulting
    increment value depends on whether you have selected a
    gridline-registered or pixel-registered grid; see Appendix B for
    details. Note: if **-R**\ *grdfile* is used then the grid spacing
    has already been initialized; use **-I** to override the values.
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]
    *west*, *east*, *south*, and *north* specify the region of interest,
    and you may specify them in decimal degrees or in
    [+-]dd:mm[:ss.xxx][W\|E\|S\|N] format. Append **r** if lower left
    and upper right map coordinates are given instead of w/e/s/n. The
    two shorthands **-Rg** and **-Rd** stand for global domain (0/360
    and -180/+180 in longitude respectively, with -90/+90 in latitude).
    Alternatively, specify the name of an existing grid file and the
    **-R** settings (and grid spacing, if applicable) are copied from
    the grid.

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
**-D**\ *resolution*\ [**+**\ ]
    Selects the resolution of the data set to use ((**f**)ull,
    (**h**)igh, (**i**)ntermediate, (**l**)ow, or (**c**)rude). The
    resolution drops off by ~80% between data sets. [Default is **l**].
    Append **+** to automatically select a lower resolution should the
    one requested not be available [abort if not found]. Note that
    because the coastlines differ in details a node in a mask file using
    one resolution is not guaranteed to remain inside [or outside] when
    a different resolution is selected.
**-N**\ *maskvalues*\ [**o**\ ]
    Sets the values that will be assigned to nodes. Values can be any
    number, including the textstring NaN. Append **o** to let nodes
    exactly on feature boundaries be considered outside [Default is
    inside]. Specify this information using 1 of 2 formats:

    **-N**\ *wet/dry*.

    **-N**\ *ocean/land/lake/island/pond*.

    [Default is 0/1/0/1/0 (i.e., 0/1)].

**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-r**
    Set pixel node registration [gridline].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Grid File Formats <#toc6>`_
----------------------------

By default **GMT** writes out grid as single precision floats in a
COARDS-complaint netCDF file format. However, **GMT** is able to produce
grid files in many other commonly used grid file formats and also
facilitates so called "packing" of grids, writing out floating point
data as 1- or 2-byte integers. To specify the precision, scale and
offset, the user should add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. See `**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.20 of
the GMT Technical Reference and Cookbook for more information.

When writing a netCDF file, the grid is stored by default with the
variable name "z". To specify another variable name *varname*, append
**?**\ *varname* to the file name. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes.

`Examples <#toc7>`_
-------------------

To set all nodes on land to NaN, and nodes over water to 1, using the
high resolution data set, do

grdlandmask -R-60/-40/-40/-30 -Dh -I5m -N1/NaN -Gland\_mask.nc -V

To make a 1x1 degree global grid with the hierarchical levels of the
nodes based on the low resolution data:

grdlandmask -R0/360/-90/90 -Dl -I1 -N0/1/2/3/4 -Glevels.nc -V

`Gshhs Information <#toc8>`_
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

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*grdmath*\ (1) <grdmath.html>`_ ,
`*grdclip*\ (1) <grdclip.html>`_ , `*psmask*\ (1) <psmask.html>`_ ,
`*psclip*\ (1) <psclip.html>`_ , `*pscoast*\ (1) <pscoast.html>`_
