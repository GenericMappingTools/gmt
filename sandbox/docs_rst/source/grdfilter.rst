*********
grdfilter
*********

grdfilter - Filter a grid in the space (or time) domain

`Synopsis <#toc1>`_
-------------------

**grdfilter** *ingrid* **-D**\ *distance\_flag*
**-F**\ *<filtertype><width>*\ [/*width2*][*mode*\ ][\ **+q**\ *quantile*]
**-G**\ *outgrid* [
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
] [ **-N**\ **i**\ \|\ **p**\ \|\ **r** ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-T** ] [
**-V**\ [*level*\ ] ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo* ]

`Description <#toc2>`_
----------------------

**grdfilter** will filter a *.nc* file in the time domain using one of
the selected convolution or non-convolution isotropic or rectangular
filters and compute distances using Cartesian or Spherical geometries.
The output *.nc* file can optionally be generated as a sub-region of the
input (via **-R**) and/or with new increment (via **-I**) or
registration (via **-T**). In this way, one may have "extra space" in
the input data so that the edges will not be used and the output can be
within one-half- width of the input edges. If the filter is low-pass,
then the output may be less frequently sampled than the input.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*ingrid*
    The grid file of points to be filtered. (See GRID FILE FORMATS
    below).
**-D**\ *distance\_flag*
    Distance *flag* tells how grid (x,y) relates to filter *width* as
    follows:

    *flag* = p: grid (px,py) with *width* an odd number of pixels;
    Cartesian distances.

    *flag* = 0: grid (x,y) same units as *width*, Cartesian distances.

    *flag* = 1: grid (x,y) in degrees, *width* in kilometers, Cartesian
    distances.

    *flag* = 2: grid (x,y) in degrees, *width* in km, dx scaled by
    cos(middle y), Cartesian distances.

    The above options are fastest because they allow weight matrix to be
    computed only once. The next three options are slower because they
    recompute weights for each latitude.

    *flag* = 3: grid (x,y) in degrees, *width* in km, dx scaled by
    cosine(y), Cartesian distance calculation.

    *flag* = 4: grid (x,y) in degrees, *width* in km, Spherical distance
    calculation.

    *flag* = 5: grid (x,y) in Mercator **-Jm**\ 1 img units, *width* in
    km, Spherical distance calculation.

**-F**\ *<filtertype><width>*\ [/*width2*][*mode*\ ]
    Sets the filter type. Choose among convolution and non-convolution
    filters. Use any *filtertype* (listed below) followed by the full
    diameter *width*. This gives an isotropic filter; append /*width2*
    for a rectangular filter (requires **-Dp** or **-D0**).

    Convolution filters are:

    (**b**) Boxcar: All weights are equal.

    (**c**) Cosine Arch: Weights follow a cosine arch curve.

    (**g**) Gaussian: Weights are given by the Gaussian function, where
    *width* is 6 times the conventional Gaussian sigma.

    Non-convolution filters are:

    (**m**) Median: Returns median value. To select another quantile
    append **+q**\ *quantile* in the 0-1 range [Default is 0.5, i.e.,
    median].

    (**p**) Maximum likelihood probability (a mode estimator): Return
    modal value. If more than one mode is found we return their average
    value. Append **-** or **+** to the filter width if you rather want
    to return the smallest or largest of the modal values.

    (**l**) Lower: Return the minimum of all values.

    (**L**) Lower: Return minimum of all positive values only.

    (**u**) Upper: Return maximum of all values.

    (**U**) Upper: Return maximum or all negative values only.

    In the case of **L**\ \|\ **U** it is possible that no data passes
    the initial sign test; in that case the filter will return 0.0.

**-G**\ *outgrid*
    *outgrid* is the output grid file of the filter. (See GRID FILE
    FORMATS below).

`Optional Arguments <#toc5>`_
-----------------------------

**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
    *x\_inc* [and optionally *y\_inc*] is the output Increment. Append
    **m** to indicate arc minutes, or **s** to indicate arc seconds. If
    the new *x\_inc*, *y\_inc* are NOT integer multiples of the old ones
    (in the input data), filtering will be considerably slower.
    [Default: Same as input.]
**-N**\ **i**\ \|\ **p**\ \|\ **r**
    Determine how NaN-values in the input grid affects the filtered
    output: Append **i** to ignore all NaNs in the calculation of
    filtered value [Default], **r** is same as **i** except if the input
    node was NaN then the output node will be set to NaN (only applies
    if both grids are co-registered), and **p** which will force the
    filtered value to be NaN if any grid-nodes with NaN-values are found
    inside the filter circle.
**-R**
    *west*, *east*, *south*, and *north* defines the Region of the
    output points. [Default: Same as input.]
**-T**
    Toggle the node registration for the output grid so as to become the
    opposite of the input grid [Default gives the same registration as
    the input grid].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
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
data. When reading grids, the format is generally automatically
recognized. If not, the same suffix can be added to input grid file
names. See `**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.20
of the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. The **?**\ *varname* suffix can also be used for output
grids to specify a variable name different from the default: "z". See
`**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.20 of the GMT
Technical Reference and Cookbook for more information, particularly on
how to read splices of 3-, 4-, or 5-dimensional grids.

`Geographical And Time Coordinates <#toc7>`_
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

`Examples <#toc8>`_
-------------------

Suppose that north\_pacific\_etopo5.nc is a file of 5 minute bathymetry
from 140E to 260E and 0N to 50N, and you want to find the medians of
values within a 300km radius (600km full width) of the output points,
which you choose to be from 150E to 250E and 10N to 40N, and you want
the output values every 0.5 degree. Using spherical distance
calculations, you need:

grdfilter north\_pacific\_etopo5.nc -Gfiltered\_pacific.nc -Fm600 -D4
-R150/250/10/40 -I0.5 -V

If we instead wanted a high-pass result then one can perform the
corresponding low-pass filter using a coarse grid interval as grdfilter
will resample the result to the same resolution as the input grid so we
can compute the residuals, e.g.,

grdfilter north\_pacific\_etopo5.nc -Gresidual\_pacific.nc -Fm-600 -D4
-R150/250/10/40 -I0.5 -V

Here, the residual\_pacific.nc grid will have the same 5 minute
resolution as the original.

`Limitations <#toc9>`_
----------------------

To use the **-D**\ 5 option the input Mercator grid must be created by
img2mercgrd using the **-C** option so the origin of the y-values is the
Equator (i.e., x = y = 0 correspond to lon = lat = 0).

`See Also <#toc10>`_
--------------------

`*gmt*\ (1) <gmt.html>`_ , `*grdfft*\ (1) <grdfft.html>`_
`*img2mercgrd*\ (1) <img2mercgrd.html>`_
