*********
gmtvector
*********

gmtvector - Basic manipulation of Cartesian vectors

`Synopsis <#toc1>`_
-------------------

**gmtvector** [ *table* ] [ **-A**\ **m**\ [*conf*\ ]\|\ *vector* ] [
**-C**\ [**i**\ \|\ **o**] ] [ **-E** ] [ **-N** ] [ **-S**\ *vector* ]
[
**-T**\ **a**\ \|\ **d**\ \|\ **D**\ \|\ **r**\ [*arg*\ \|\ **s**\ \|\ **x**]
] [ **-V**\ [*level*\ ] ] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**gmtvector** reads either (x, y), (x, y, z), (r, theta) or (lon, lat)
[or (lat,lon); see **-:**] coordinates from the first 2-3 columns on
standard input [or *infiles*]. If **-fg** is selected and only two items
are read (i.e., lon, lat) then these coordinates are converted to
Cartesian three-vectors on the unit sphere. Otherwise we expect (r,
theta) unless **-Ci** is in effect. If no file is found we expect a
single vector to be given as argument to **-A**; this argument will also
be interpreted as an x/y[/z], lon/lat, or r/theta vector. The input
vectors (or the one provided via **-A**) are denoted the prime
vector(s). Several standard vector operations (angle between vectors,
cross products, vector sums, and vector rotations) can be selected; most
require a single second vector, provided via **-S**. The output vectors
will be converted back to (lon, lat) or (r, theta) unless **-Co** is set
which requests (x, y[, z]) Cartesian coordinates.

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
    One or more ASCII [or binary, see **-bi**\ [*ncols*\ ][*type*\ ]]
    file containing lon,lat [lat,lon if **-:**] values in the first 2
    columns (if **-fg** is given) or (r, theta), or perhaps (x, y[, z])
    if **-Ci** is given). If no file is specified, **gmtvector**, will
    read from standard input.
**-A**\ **m**\ [*conf*\ ]\|\ *vector*
    Specify a single, primary vector instead of reading *infiles*; see
    *infiles* for possible vector formats. Alternatively, append **m**
    to read *infiles* and set the single, primary vector to be the mean
    resultant vector first. We also compute the confidence ellipse for
    the mean vector (azimuth of major axis, major axis, and minor axis;
    for geographic data the axes will be reported in km). You may
    optionally append the confidence level in percent [95]. These three
    parameters are reported in the final three output columns.
**-C**\ [**i**\ \|\ **o**]
    Select Cartesian coordinates on input and output. Append **i** for
    input only or **o** for output only; otherwise both input and output
    will be assumed to be Cartesian [Default is polar r/theta for 2-D
    data and geographic lon/lat for 3-D].
**-E**
    Convert input geographic coordinates from geodetic to geocentric and
    output geographic coordinates from geocentric to geodetic. Ignored
    unless **-fg** is in effect, and is bypassed if **-C** is selected.
**-N**
    Normalize the resultant vectors prior to reporting the output [No
    normalization]. This only has an effect if **-Co** is selected.
**-S**\ [*vector*\ ]
    Specify a single, secondary vector in the same format as the first
    vector. Required by operations in **-T** that need two vectors
    (average, bisector, dot product, cross product, and sum).
**-T**\ **a**\ \|\ **d**\ \|\ **D**\ \|\ **s**\ \|\ **r**\ [*arg*\ \|\ **x**]
    Specify the vector transformation of interest. Append **a** for
    average, **b** for the pole of the two points bisector, **d** for
    dot product (use **D** to get angle in degrees between the two
    vectors), **r**\ *par* for vector rotation (here, *par* is a single
    angle for 2-D Cartesian data and *lon/lat/angle* for a 3-D rotation
    pole and angle), **s** for vector sum, and **x** for cross-product.
    If **-T** is not given then no transformation takes place; the
    output is determined by other options such as **-A**, **-C**,
    **-E**, and **-N**.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2 or 3 input columns].
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

Suppose you have a file with lon, lat called points.txt. You want to
compute the spherical angle between each of these points and the
location 133/34. Try

gmtvector points.txt -S133/34 -TD -fg > angles.txt

To rotate the same points 35 degrees around a pole at 133/34, and output
Cartesian 3-D vectors, use

gmtvector points.txt -Tr133/34 -Co -fg > reconstructed.txt

To compute the cross-product between the two Cartesian vectors 0.5/1/2
and 1/0/0.4, and normalizing the result, try

gmtvector -A0.5/1/2 -Tx -S1/0/0.4 -N -C > cross.txt

To rotate the 2-D vector, given in polar form as r = 2 and theta = 35,
by an angle of 120, try

gmtvector -A2/35 -Tr120 > rotated.txt

To find the mid-point along the great circle connecting the points
123/35 and -155/-30, use

gmtvector -A123/35 -S-155/-30 -Ta -fg > midpoint.txt

To find the mean location of the geographical points listed in
points.txt, with its 99% confidence ellipse, use

gmtvector points.txt -Am99 -fg > centroid.txt

`Rotations <#toc8>`_
--------------------

For more advanced 3-D rotations as used in plate tectonic
reconstructions, see the GMT "spotter" supplement.

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*project*\ (1) <project.html>`_ ,
`*mapproject*\ (1) <mapproject.html>`_
