****************
sphtriangulate
****************

sphtriangulate - Delaunay or Voronoi construction of spherical lon,lat
data

`Synopsis <#toc1>`_
-------------------

**sphtriangulate** [ *table* ] [ **-A** ] [ **-C** ] [ **-D** ] [
**-L**\ *unit* ] [ **-N**\ *nfile* ] [ **-Q**\ **d**\ \|\ **v** ] [
**-T** ] [ **-V**\ [*level*\ ] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**sphtriangulate** reads one or more ASCII [or binary] files (or
standard input) containing lon, lat and performs a spherical Delaunay
triangulation, i.e., it find how the points should be connected to give
the most equilateral triangulation possible on the sphere. Optionally,
you may choose **-Qv** which will do further processing to obtain the
Voronoi polygons. Normally, either set of polygons will be written as
fillable segment output; use **-T** to write unique arcs instead. As an
option, compute the area of each triangle or polygon. The algorithm used
is STRIPACK.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

None.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    data table file(s) holding a number of data columns. If no tables
    are given then we read from standard input.
**-A**
    Compute the area of the spherical triangles (**-Qd**) or polygons
    (**-Qv**) and write the areas (in chosen units; see **-L**) in the
    output segment headers [no areas calculated].
**-C**
    For large data set you can save some memory (at the expense of more
    processing) by only storing one form of location coordinates
    (geographic or Cartesian 3-D vectors) at any given time, translating
    from one form to the other when necessary [Default keeps both arrays
    in memory].
**-D**
    Used with **-m** to skip the last (repeated) input vertex at the end
    of a closed segment if it equals the first point in the segment.
    Requires **-m** [Default uses all points].
**-L**\ *unit*
    Specify the unit used for distance and area calculations. Choose
    among **e** (m), **k** (km), **m** (mile), **n** (nautical mile), or
    **d** (spherical degree). A spherical approximation is used unless
    **PROJ\_ELLIPSOID** is set to an actual ellipsoid, in which case we
    convert latitudes to authalic latitudes before calculating areas.
    When degree is selected the areas are given in steradians.
**-N**\ *nfile*
    Write the information pertaining to each polygon (for Delaunay: the
    three node number and the triangle area (if **-A** was set); for
    Voronoi the unique node lon, lat and polygon area (if **-A** was
    set)) to a separate file. This information is also encoded in the
    segment headers of ASCII output files]. Required if binary output is
    needed.
**d**\ \|\ **v**
    Select between **d**\ elaunay or **v**\ oronoi mode [Delaunay].
**-T**
    Write the unique arcs of the construction [Default writes fillable
    triangles or polygons]. When used with **-A** we store arc lenght in
    the segment header in chosen unit (see **-L**).
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output. [Default is same as input].
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
(\*)
    Select input columns.
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

To triangulate the points in the file testdata.txt, and make a Voronoi
diagram via **psxy**, use

**sphtriangulate** testdata.txt **-Qv** \| **psxy** **-Rg**
**-JG**\ 30/30/6i **-m** OPR(L) **-P** **-W**\ 1p **-B**\ 0g30 \| gv -

To compute the optimal Delaunay triangulation network based on the
multiple segment file globalnodes.d and save the area of each triangle
in the header record, try

**sphtriangulate** globalnodes.d **-m** **-Qd** **-A** > global\_tri.d

`See Also <#toc9>`_
-------------------

`*GMT*\ (1) <GMT.html>`_ , `*triangulate*\ (1) <triangulate.html>`_
`*sphinterpolate*\ (1) <sphinterpolate.html>`_
`*sphdistance*\ (1) <sphdistance.html>`_

`References <#toc10>`_
----------------------

Renka, R, J., 1997, Algorithm 772: STRIPACK: Delaunay Triangulation and
Voronoi Diagram on the Surface of a Sphere, *AMC Trans. Math. Software*,
**23**\ (3), 416-434.
