********
grdtrack
********

grdtrack - Sample grids at specified (x,y) locations

`Synopsis <#toc1>`_
-------------------

**grdtrack** [ *xyfile* ] **-G**\ *grd1* **-G**\ *grd2* ... [
**-A**\ **m**\ \|\ **p** ] [
**-C**\ *length*/*ds*\ [*spacing*\ ][**+a**\ ] ] [ **-D**\ *dfile* ] [
**-L**\ *flag* ] [ **-N** ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [
**-S**\ *method*/*modifiers* ][ **-V**\ [*level*\ ] ] [ **-Z** ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [
**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-s**\ [*cols*\ ][\ **a**\ \|\ **r**]
] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**grdtrack** reads one or more grid files (or a Sandwell/Smith IMG
files) and a table (from file or standard input) with (x,y) [or
(lon,lat)] positions in the first two columns (more columns may be
present). It interpolates the grid(s) at the positions in the table and
writes out the table with the interpolated values added as (one or more)
new columns. Alternatively (**-C**), the input is considered to be
line-segments and we create orthogonal cross-profiles at each data point
or with an equidistant separation and sample the grid(s) along these
profiles. A bicubic [Default], bilinear, B-spline or nearest-neighbor
(see **-n**) interpolation is used, requiring boundary conditions at the
limits of the region (see **-L**).

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-G**\ *gridfile*
    *grdfile* is a 2-D binary grid file with the function f(x,y). If the
    specified grid is in Sandwell/Smith Mercator format you must append
    a comma-separated list of arguments that includes a scale to
    multiply the data (usually 1 or 0.1), the mode which stand for the
    following: (0) Img files with no constraint code, returns data at
    all points, (1) Img file with constraints coded, return data at all
    points, (2) Img file with constraints coded, return data only at
    constrained points and NaN elsewhere, `and (3) <and.html>`_ Img file
    with constraints coded, return 1 at constraints and 0 elsewhere, and
    optionally the max latitude in the IMG file [80.738]. You may repeat
    **-G** as many times as you have grids you wish to sample. The grids
    are sampled and results are output in the order given. (See GRID
    FILE FORMAT below.)

`Optional Arguments <#toc5>`_
-----------------------------

*xyfile*
    This is an ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    file where the first 2 columns hold the (x,y) positions where the
    user wants to sample the 2-D data set.
**-A**\ **m**\ \|\ **p**
    For spherical surface resampling we resample along great circle
    arcs. Alternatively, use **-Am** to resample by first following a
    meridian, then a parallel. Or use **-Ap** to start following a
    parallel, then a meridian. (This can be practical for resampling
    lines along parallels, for example). Ignored unless **-C** is used.
**-C**\ *length*\ **/**\ *ds*\ [**/**\ *spacing*]
    Use input line segments to create an equidistant and (optionally)
    equally-spaced set of crossing profiles along which we sample the
    grid(s) [Default simply samples the grid(s) at the input locations].
    Specify two length scales that control how the sampling is done:
    *length* sets the full length of each cross-profile, while *ds* is
    the distance increment along each cross-profile. Optionally, append
    **/**\ *spacing* for an equidistant spacing between cross-profiles
    [Default erects cross-profiles at the input coordinates]. By
    default, all cross-profiles have the same direction. Append **+a**
    to alternate the direction of cross-profiles. Append suitable units
    for each length scale (See UNITS below). The output columns will be
    *lon*, *lat*, *dist*, *azimuth*, *z1*, *z2*, ... (sampled value for
    each grid)
**-D**\ *dfile*
    In concert with **-C** we can save the (possibly resampled) original
    lines to the file *dfile* [Default only saves the cross-profiles].
    The columns will be *lon*, *lat*, *dist*, *azimuth*, *z1*, *z2*, ...
    (sampled value for each grid)
**-L**\ *flag*
    Boundary condition *flag* may be *x* or *y* or *xy* indicating data
    is periodic in range of x or y or both set by **-R**, or *flag* may
    be *g* indicating geographical conditions (x and y are lon and lat).
    [Default uses "natural" conditions (second partial derivative normal
    to edge is zero) unless the grid is automatically recognized as
    periodic.]
**-N**
    Do *not* skip points that fall outside the domain of the grid(s)
    [Default only output points within grid domain].
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
**-S**\ *method*/*modifiers*
    In conjunction with **-C**, compute a single stacked profile from
    all profiles across each segment. Append how stacking should be
    computed: **a** = mean (average), **m** = median, **p** = mode
    (maximum likelihood), **l** = lower, **L** = lower but only consider
    positive values, **u** = upper, **U** = upper but only consider
    negative values [**a**\ ]. The *modifiers* control the output;
    choose one or more among these choices: **+a** : Append stacked
    values to all cross-profiles. **+d** : Append stack deviations to
    all cross-profiles. **+d** : Append data residuals (data - stack) to
    all cross-profiles. **+s**\ [*file*\ ] : Save stacked profile to
    *file* [grdtrack\_stacked\_profile.txt]. **+c**\ *fact* : Compute
    envelope on stacked profile as +/- *fact*\ \*\ *deviation* [2].
    Notes: (1) Deviations depend on *method* and are st.dev (**a**), L1
    scale (**e** and **p**), or half-range (upper-lower)/2. (2) The
    stacked profile file contains 1 plus groups of 4-6 columns, one
    group for each sampled grid. The first column holds cross distance,
    while the first 4 in a group hold stacked value, deviation, min
    value, and max value. If *method* is one of
    **a**\ \|\ **m**\ \|\ **p** then we also write the lower and upper
    confidence bounds (see **+c**). When one or more of **+a**, **+d**,
    and **+r** are used then we append the results to the end of each
    row for all cross-profiles. The order is always stacked value
    (**+a**), followed by deviations (**+d**) and residuals (**+r**).
    When more than one grid is sampled this sequence of 1-3 columns are
    repeated for each grid.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-Z**
    Only write out the sampled z-values [Default writes all columns].
**-:**
    Toggles between (longitude,latitude) and (latitude,longitude)
    input/output. [Default is (longitude,latitude)].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output. [Default is one more than input].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
(\*)
    Determine data gaps and line breaks.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
(\*)
    Select interpolation mode for grids.
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns.
**-s**\ [*cols*\ ][\ **a**\ \|\ **r**] (\*)
    Set handling of NaN records.
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

For map distance unit, append *unit* **d** for arc degree, **m** for arc
minute, and **s** for arc second, or **e** for meter [Default], **f**
for foot, **k** for km, **M** for statute mile, **n** for nautical mile,
and **u** for US survey foot. By default we compute such distances using
a spherical approximation with great circles. Prepend **-** to a
distance (or the unit is no distance is given) to perform "Flat Earth"
calculations (quicker but less accurate) or prepend **+** to perform
exact geodesic calculations (slower but more accurate).

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

`Grid File Formats <#toc8>`_
----------------------------

**GMT** is able to recognize many of the commonly used grid file
formats, as well as the precision, scale and offset of the values
contained in the grid file. When **GMT** needs a little help with that,
you can add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. See `**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.17 of
the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. See `**grdreformat**\ (1) <grdreformat.html>`_ and
Section 4.18 of the GMT Technical Reference and Cookbook for more
information, particularly on how to read splices of 3-, 4-, or
5-dimensional grids.

`Hints <#toc9>`_
----------------

If an interpolation point is not on a node of the input grid, then a NaN
at any node in the neighborhood surrounding the point will yield an
interpolated NaN. Bicubic interpolation [default] yields continuous
first derivatives but requires a neighborhood of 4 nodes by 4 nodes.
Bilinear interpolation [**-n**\ ] uses only a 2 by 2 neighborhood, but
yields only zeroth-order continuity. Use bicubic when smoothness is
important. Use bilinear to minimize the propagation of NaNs, or lower
*threshold*.

`Examples <#toc10>`_
--------------------

To sample the file hawaii\_topo.nc along the SEASAT track track\_4.xyg
(An ASCII table containing longitude, latitude, and SEASAT-derived
gravity, preceded by one header record):

grdtrack track\_4.xyg -Ghawaii\_topo.nc -h > track\_4.xygt

To sample the Sandwell/Smith IMG format file topo.8.2.img (2 minute
predicted bathymetry on a Mercator grid) and the Muller et al age grid
age.3.2.nc along the lon,lat coordinates given in the file
cruise\_track.xy, try

grdtrack cruise\_track.xy -Gtopo.8.2.img,1,1 -Gage.3.2.nc > depths-age.d

To sample the Sandwell/Smith IMG format file grav.18.1.img (1 minute
free-air anomalies on a Mercator grid) along 100-km-long cross-profiles
that are orthogonal to the line segment given in the file track.xy,
erecting cross-profiles every 25 km and sampling the grid every 3 km,
try

grdtrack track.xy -Ggrav.18.1.img,0.1,1 -C100k/3k/25k > xprofiles.d

`See Also <#toc11>`_
--------------------

`*gmt*\ (1) <gmt.html>`_ , `*surface*\ (1) <surface.html>`_ ,
`*sample1d*\ (1) <sample1d.html>`_
