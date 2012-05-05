************
nearneighbor
************


nearneighbor - Grid table data using a "Nearest neighbor" algorithm

`Synopsis <#toc1>`_
-------------------

**nearneighbor** [ *table* ] **-G**\ *out\_grdfile*
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-N**\ *sectors*\ [/*min\_sectors*]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ]
**-S**\ *search\_radius*\ [*unit*\ ] [ **-E**\ *empty* ] [
**-V**\ [*level*\ ] ] [ **-W** ] [ **-bi**\ [*ncol*\ ][**t**\ ] ] [
**-f**\ *colinfo* ] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-n**\ [**+b**\ *BC*] ] [ **-r** ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**nearneighbor** reads arbitrarily located (x,y,z[,w]) triples
[quadruplets] from standard input [or *table*] and uses a nearest
neighbor algorithm to assign an average value to each node that have one
or more points within a radius centered on the node. The average value
is computed as a weighted mean of the nearest point from each sector
inside the search radius. The weighting function used is w(r) = 1 / (1 +
d ^ 2), where d = 3 \* r / search\_radius and r is distance from the
node. This weight is modulated by the observation points’ weights [if
supplied].

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-G**\ *out\_grdfile*
    Give the name of the output grid file.
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
**-N**\ *sectors*\ [/*min\_sectors*]
    The circular area centered on each node is divided into *sectors*
    sectors. Average values will only be computed if there is at least
    one value inside at least *min\_sectors* of the sectors for a given
    node. Nodes that fail this test are assigned the value NaN (but see
    **-E**). If *min\_sectors* is omitted, each sector needs to have at
    least one value inside it. [Default is quadrant search with 50%
    coverage, i.e., *sectors* = 4 and *min\_sectors* = 2]. Note that
    only the nearest value per sector enters into the averaging, not all
    values inside the circle.
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
**-S**\ *search\_radius*\ [*unit*\ ]
    Sets the *search\_radius* that determines which data points are
    considered close to a node. Append the distance unit (see UNITS).

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    3 [or 4, see **-W**] column ASCII file(s) [or binary, see
    **-bi**\ [*ncol*\ ][**t**\ ]] holding (x,y,z[,w]) data values. If no
    file is specified, **nearneighbor** will read from standard input.
**-E**\ *empty*
    Set the value assigned to empty nodes [NaN].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-W**
    Input data have a 4th column containing observation point weights.
    These are multiplied with the geometrical weight factor to determine
    the actual weights used in the calculations.
**-bi**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary input. [Default is 3 (or 4 if **-W** is set) columns].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*] (\*)
    Select input columns.
**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
    Append **+b**\ *BC* to set any boundary conditions to be used,
    adding **g** for geographic, **p** for periodic, or **n** for
    natural boundary conditions. For the latter two you may append **x**
    or **y** to specify just one direction, otherwise both are assumed.
    [Default is geographic if grid is geographic].
**-r**
    Set pixel node registration [gridline]. Not used with binary data.
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

To create a gridded data set from the file seaMARCII\_bathy.lon\_lat\_z
using a 0.5 min grid, a 5 km search radius, using an octant search, and
set empty nodes to -9999:

nearneighbor seaMARCII\_bathy.lon\_lat\_z -R242/244/-22/-20 -I0.5m
-E-9999 -Gbathymetry.nc **-S**\ 5\ **k** **-N**\ 8

To make a global grid file from the data in geoid.xyz using a 1 degree
grid, a 200 km search radius, spherical distances, using an quadrant
search, and set nodes to NaN only when fewer than two quadrants contain
at least one value:

nearneighbor geoid.xyz -R0/360/-90/90 -I1 -Lg -Ggeoid.nc -S200k -N4/2

`See Also <#toc9>`_
-------------------

`*blockmean*\ (1) <blockmean.1.html>`_ ,
`*blockmedian*\ (1) <blockmedian.1.html>`_ ,
`*blockmode*\ (1) <blockmode.1.html>`_ , `*gmt*\ (1) <gmt.1.html>`_ ,
`*surface*\ (1) <surface.1.html>`_ ,
`*triangulate*\ (1) <triangulate.1.html>`_

