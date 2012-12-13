***********
triangulate
***********

triangulate - Do optimal (Delaunay) triangulation and gridding of
Cartesian table data [method]

`Synopsis <#toc1>`_
-------------------

**triangulate** [ *table* ] [ **-Dx**\ \|\ **y** ] [ **-E**\ *empty* ] [
**-G**\ *grdfile* ] [
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
] [ **-J**\ *parameters* ] [ **-M** ] [ **-Q** ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-S** ] [
**-V**\ [*level*\ ] ] [ **-Z** ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-r** ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**triangulate** reads one or more ASCII [or binary] files (or standard
input) containing x,y[,z] and performs Delaunay triangulation, i.e., it
find how the points should be connected to give the most equilateral
triangulation possible. If a map projection (give **-R** and **-J**) is
chosen then it is applied before the triangulation is calculated. By
default, the output is triplets of point id numbers that make up each
triangle and is written to standard output. The id numbers refer to the
points position (line number, starting at 0 for the first line) in the
input file. As an option, you may choose to create a multiple segment
file that can be piped through **psxy** to draw the triangulation
network. If **-G** **-I** are set a grid will be calculated based on the
surface defined by the planar triangles. The actual algorithm used in
the triangulations is either that of Watson [1982] [Default] or Shewchuk
[1996] (if installed; type **triangulate -** to see which method is
selected). This choice is made during the **GMT** installation.

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
**-Dx**\ \|\ **y**
    Take either the *x*- or *y*-derivatives of surface represented by
    the planar facets (only used when **-G** is set).
**-E**\ *empty*
    Set the value assigned to empty nodes when **-G** is set [NaN].
**-G**\ *grdfile*
    Use triangulation to grid the data onto an even grid (specified with
    **-R** **-I**). Append the name of the output grid file. The
    interpolation is performed in the original coordinates, so if your
    triangles are close to the poles you are better off projecting all
    data to a local coordinate system before using **triangulate** (this
    is true of all gridding routines).
**-I**
    *x\_inc* [and optionally *y\_inc*] sets the grid size for optional
    grid output (see **-G**). Append **m** to indicate arc minutes or
    **s** to indicate arc seconds.
**-J**\ *parameters* (\*)
    Select map projection.
**-M**
    Output triangulation network as multiple line segments separated by
    a segment header record.
**-Q**
    Output the edges of the Voronoi cells instead [Default is Delaunay
    triangle edges]. Requires **-R** and is only available if linked
    with the Shewchuk [1996] library. Note that **-Z** is ignored on
    output.
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
**-S**
    Output triangles as polygon segments separated by a segment header
    record. Requires Delaunay triangulation.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-Z**
    Controls whether we read (x,y) or (x,y,z) data and if z should be
    output when **-M** or **-S** are used [Read (x,y) only].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output. [Default is same as input]. Node ids are
    stored as double triplets.
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-r**
    Set pixel node registration [gridline]. Only valid with **-G**).
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

To triangulate the points in the file samples.xyz, store the triangle
information in a binary file, and make a grid for the given area and
spacing, use

triangulate samples.xyz -bo -R0/30/0/30 -I2 -Gsurf.nc > samples.ijk

To draw the optimal Delaunay triangulation network based on the same
file using a 15-cm-wide Mercator map, use

triangulate samples.xyz -M -R-100/-90/30/34 **-JM**\ 15\ **c** \| psxy
-R-100/-90/30/34 **-JM**\ 15\ **c** -W0.5p -B1 > network.ps

To instead plot the Voronoi cell outlines, try
 triangulate samples.xyz -M -Q -R-100/-90/30/34 **-JM**\ 15\ **c** \|
psxy -R-100/-90/30/34 **-JM**\ 15\ **c** -W0.5p -B1 > cells.ps

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*pscontour*\ (1) <pscontour.html>`_

`References <#toc10>`_
----------------------

Watson, D. F., 1982, Acord: Automatic contouring of raw data, *Comp. &
Geosci.*, **8**, 97-101.

Shewchuk, J. R., 1996, Triangle: Engineering a 2D Quality Mesh Generator
and Delaunay Triangulator, First Workshop on Applied Computational
Geometry (Philadelphia, PA), 124-133, ACM, May 1996.

www.cs.cmu.edu/~quake/triangle.html
