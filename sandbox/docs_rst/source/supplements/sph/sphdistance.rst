***********
sphdistance
***********


sphdistance - Make grid of distances to nearest points on a sphere

`Synopsis <#toc1>`_
-------------------

**sphdistance** [ *table* ] **-G**\ *grdfile* [ **-C** ] [ **-D** ] [
**-E** ] [
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
] [ **-L**\ *unit* ] [ **-Q**\ *voronoi.d* ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-V**\ [*level*\ ]
] [ **-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-r** ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**sphdistance** reads one or more ASCII [or binary] files (or standard
input) containing lon, lat and performs the construction of Voronoi
polygons. These polygons are then processed to calculate the nearest
distance to each node of the lattice and written to the specified grid.
The Voronoi algorithm used is STRIPACK. As an option, you may provide
pre-calculated Voronoi polygon file in the format written by
**sphtriangulate**, thus bypassing the memory- and time-consuming
triangularization.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-G**\ *grdfile*
    Name of the output grid to hold the computed distances.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncol*\ ][**t**\ ]) data
    table file(s) holding a number of data columns. If no tables are
    given then we read from standard input.
**-C**
    For large data set you can save some memory (at the expense of more
    processing) by only storing one form of location coordinates
    (geographic or Cartesian 3-D vectors) at any given time, translating
    from one form to the other when necessary [Default keeps both arrays
    in memory]. Not applicable with **-Q**.
**-D**
    Used with **-m** to skip the last (repeated) input vertex at the end
    of a closed segment if it equals the first point in the segment.
    Requires **-m** [Default uses all points].
**-E**
    Instead of computing distances, return the ID numbers of the Voronoi
    polygons that each grid node is inside [Default computes distances].
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
**-L**\ *unit*
    Specify the unit used for distance calculations. Choose among **d**
    (spherical degree), **e** (m), **f** (feet), **k** (km), **M**
    (mile), or **n** (nautical mile). A spherical approximation is used
    unless **PROJ\_ELLIPSOID** is set to an actual ellipsoid. **-N**
    Read the information pertaining to each Voronoi polygon (the unique
    node lon, lat and polygon area) from a separate file [Default
    acquires this information from the ASCII segment headers of the
    output file]. Required if binary input via **-Q** is used.
**-Q**\ *voronoi.d*
    Append the name of a file with pre-calculated Voronoi polygons
    [Default performs the Voronoi construction on input data]. For
    binary data **-bi**\ [*ncol*\ ][**t**\ ] you must specify the node
    information separately (via **-N**).
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
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-bi**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-bo**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary output. [Default is same as input].
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*] (\*)
    Select input columns.
**-r**
    Set pixel node registration [gridline].
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Ascii Format Precision <#toc6>`_
---------------------------------

The ASCII output formats of numerical data are controlled by parameters
in your **gmt.conf** file. Longitude and latitude are formatted
according to **FORMAT\_GEO\_OUT**, whereas other values are formatted
according to **FORMAT\_FLOAT\_OUT**. Be aware that the format in effect
can lead to loss of precision in the output, which can lead to various
problems downstream. If you find the output is not written with enough
precision, consider switching to binary output (**-bo** if available) or
specify more decimals using the **FORMAT\_FLOAT\_OUT** setting.

`Grid Values Precision <#toc7>`_
--------------------------------

Regardless of the precision of the input data, GMT programs that create
grid files will internally hold the grids in 4-byte floating point
arrays. This is done to conserve memory and furthermore most if not all
real data can be stored using 4-byte floating point values. Data with
higher precision (i.e., double precision values) will lose that
precision once GMT operates on the grid or writes out new grids. To
limit loss of precision when processing data you should always consider
normalizing the data prior to processing.

`Examples <#toc8>`_
-------------------

To construct Voronoi polygons from the points in the file testdata.txt
and then calculate distances from the data to a global 1x1 degree grid,
use

**sphdistance** testdata.txt **-Rg** **-I**\ 1 **-G**\ globedist.nc

To generate the same grid in two steps using **sphtriangulate**
separately, try

**sphtriangulate** testdata.txt **-Qv** > voronoi.d
**sphdistance** **-Q**\ voronoi.d **-Rg** **-I**\ 1
**-G**\ globedist.nc

`See Also <#toc9>`_
-------------------

`*GMT*\ (1) <GMT.1.html>`_ ,
`*sphinterpolate*\ (1) <sphinterpolate.1.html>`_
`*sphtriangulate*\ (1) <sphtriangulate.1.html>`_
`*triangulate*\ (1) <triangulate.1.html>`_

`References <#toc10>`_
----------------------

Renka, R, J., 1997, Algorithm 772: STRIPACK: Delaunay Triangulation and
Voronoi Diagram on the Surface of a Sphere, *AMC Trans. Math. Software*,
**23**\ (3), 416-434.

