*******
grdmask
*******

grdmask - Create mask grid from polygons or point coverage

`Synopsis <#toc1>`_
-------------------

**grdmask** *pathfiles* **-G**\ *mask\_grd\_file*]
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [
**-A**\ [**m**\ \|\ **p**] ] [
**-N**\ [**i**\ \|\ **I**\ \|\ **p**\ \|\ **P**]\ *values* ] [
**-S**\ *search\_radius*\ [*unit*\ ] ] [ **-V**\ [*level*\ ] ] [
**-bi**\ [*ncols*\ ][*type*\ ] ] [ **-f**\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-r** ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**grdmask** can operate in two different modes. 1. It reads one or more
xy-files that each define a closed polygon. The nodes defined by the
specified region and lattice spacing will be set equal to one of three
possible values depending on whether the node is outside, on the polygon
perimeter, or inside the polygon. The resulting mask may be used in
subsequent operations involving **grdmath** to mask out data from
polygonal areas. 2. The xy-files simply represent data point locations
and the mask is set to the inside or outside value depending on whether
a node is within a maximum distance from the nearest data point. If the
distance specified is zero then only the nodes nearest each data point
are considered "inside".

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*pathfiles*
    The name of 1 or more ASCII [or binary, see
    **-bi**\ [*ncols*\ ][*type*\ ]] files holding the polygon(s) or data
    points.
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
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ [**m**\ \|\ **p**]
    If the input data are geographic (as indicated by **-fi**) then the
    sides in the polygons will be approximated by great circle arcs.
    When using the **-A** sides will be regarded as straight lines.
    Alternatively, append **m** to have sides first follow meridians,
    then parallels. Or append **p** to first follow parallels, then
    meridians.
**-N**\ [**z**\ \|\ **Z**\ \|\ **p**\ \|\ **P**]\ *values*
    Sets the *out/edge/in* that will be assigned to nodes that are
    *out*\ side the polygons, on the *edge*, or *in*\ side. Values can
    be any number, including the textstring NaN [Default is 0/0/1].
    Optionally, use **Nz** to set polygon insides to the z-value
    obtained from the data (either segment header **-Z**\ *zval*,
    **-L**\ *header* or via **-a**\ Z=\ *name*); use **-NZ** to consider
    the polygon boundary as part of the inside. Alternatively, use
    **-Np** to use a running number as polygon ID; optionally append
    start of the sequence [0]. Here, **-NP** includes the polygon
    perimeter as inside.
**-S**\ *search\_radius*\ [*unit*\ ]
    Set nodes depending on their distance from the nearest data point.
    Nodes within *radius* [0] from a data point are considered inside;
    append a distance unit (see UNITS). If *radius* is given as **z**
    then we instead read individual radii from the 3rd input column. If
    **-S** is not set then we consider the input data to define closed
    polygon(s) instead.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
(\*)
    Determine data gaps and line breaks.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
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

`Grid File Formats <#toc7>`_
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

`Geographical And Time Coordinates <#toc8>`_
--------------------------------------------

When the output grid type is netCDF, the coordinates will be labeled
"longitude", "latitude", or "time" based on the attributes of the input
data or grid (if any) or on the **-f** or **-R** options. For example,
both **-f0x** **-f1t** and **-R**\ 90w/90e/0t/3t will result in a
longitude/time grid. When the x, y, or z coordinate is time, it will be
stored in the grid as relative time since epoch as specified by
**TIME\_UNIT** and **TIME\_EPOCH** in the **gmt.conf** file or on the
command line. In addition, the **unit** attribute of the time variable
will indicate both this unit and epoch.

`Examples <#toc9>`_
-------------------

To set all nodes inside and on the polygons coastline\_\*.xy to 0, and
outside points to 1, do

grdmask coastline\_\*.xy -R-60/-40/-40/-30 -I5m -N1/0/0
-Gland\_mask.nc=nb -V

To set nodes within 50 km of data points to 1 and other nodes to NaN, do

grdmask data.xyz -R-60/-40/-40/-30 -I5m -NNaN/1/1 -S50k
-Gdata\_mask.nc=nb -V

To assign polygon IDs to the gridnodes using the insides of the polygons
in plates.gmt, based on the attribute POL\_ID, do

grdmask plates.gmt -R-40/40/-40/40 -I2m -Nz -Gplate\_IDs.nc -aZ=POL\_ID
-V

Same exercise, but instead compute running polygon IDs starting at 100,
do

grdmask plates.gmt -R-40/40/-40/40 -I2m -Np100 -Gplate\_IDs.nc -V

`See Also <#toc10>`_
--------------------

`*gmt*\ (1) <gmt.html>`_ , `*grdlandmask*\ (1) <grdlandmask.html>`_ ,
`*grdmath*\ (1) <grdmath.html>`_ , `*grdclip*\ (1) <grdclip.html>`_ ,
`*psmask*\ (1) <psmask.html>`_ , `*psclip*\ (1) <psclip.html>`_
