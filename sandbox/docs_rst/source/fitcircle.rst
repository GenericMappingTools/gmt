*********
fitcircle
*********

fitcircle - find mean position and pole of best-fit great [or small]
circle to points on a sphere.

`Synopsis <#toc1>`_
-------------------

**fitcircle** [ *table* ] **-L**\ *norm* [ **-S**\ [*lat*\ ] ] [
**-V**\ [*level*\ ] ] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**fitcircle** reads lon,lat [or lat,lon] values from the first two
columns on standard input [or *xyfile*]. These are converted to
Cartesian three-vectors on the unit sphere. Then two locations are
found: the mean of the input positions, and the pole to the great circle
which best fits the input positions. The user may choose one or both of
two possible solutions to this problem. The first is called **-L1** and
the second is called **-L2**. When the data are closely grouped along a
great circle both solutions are similar. If the data have large
dispersion, the pole to the great circle will be less well determined
than the mean. Compare both solutions as a qualitative check.

The **-L1** solution is so called because it approximates the
minimization of the sum of absolute values of cosines of angular
distances. This solution finds the mean position as the Fisher average
of the data, and the pole position as the Fisher average of the
cross-products between the mean and the data. Averaging cross-products
gives weight to points in proportion to their distance from the mean,
analogous to the "leverage" of distant points in linear regression in
the plane.

The **-L2** solution is so called because it approximates the
minimization of the sum of squares of cosines of angular distances. It
creates a 3 by 3 matrix of sums of squares of components of the data
vectors. The eigenvectors of this matrix give the mean and pole
locations. This method may be more subject to roundoff errors when there
are thousands of data. The pole is given by the eigenvector
corresponding to the smallest eigenvalue; it is the least-well
represented factor in the data and is not easily estimated by either
method.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-L**\ *norm*
    Specify the desired *norm* as 1 or 2, or use **-L** or **-L3** to
    see both solutions.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII [or binary, see **-bi**\ [*ncols*\ ][*type*\ ]]
    files containing lon,lat [or lat,lon; see
    **-:**\ [**i**\ \|\ **o**]] values in the first 2 columns. If no
    file is specified, **fitcircle** will read from standard input.
**-S**\ [*lat*\ ]
    Attempt to fit a small circle instead of a great circle. The pole
    will be constrained to lie on the great circle connecting the pole
    of the best-fit great circle and the mean location of the data.
    Optionally append the desired fixed latitude of the small circle
    [Default will determine the latitude].
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
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns.
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

`Examples <#toc7>`_
-------------------

Suppose you have lon,lat,grav data along a twisty ship track in the file
ship.xyg. You want to project this data onto a great circle and resample
it in distance, in order to filter it or check its spectrum. Do the
following:

fitcircle ship.xyg -L2

project ship.xyg -Cox/oy -Tpx/py -S -Fpz \| sample1d -S-100 -I1 >
output.pg

Here, *ox*/*oy* is the lon/lat of the mean from **fitcircle**, and
*px*/*py* is the lon/lat of the pole. The file output.pg has distance,
gravity data sampled every 1 km along the great circle which best fits
ship.xyg

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*project*\ (1) <project.html>`_ ,
`*sample1d*\ (1) <sample1d.html>`_
