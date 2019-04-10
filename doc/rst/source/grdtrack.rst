.. index:: ! grdtrack

********
grdtrack
********

.. only:: not man

    Sample grids at specified (x,y) locations

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdtrack** [ *xyfile* ] |-G|\ *grd1* |-G|\ *grd2* ...
[ |-A|\ **f**\ \|\ **p**\ \|\ **m**\ \|\ **r**\ \|\ **R**\ [**+l**] ]
[ |-C|\ *length*\ [**u**]/\ *ds*\ [*/spacing*][**+a**][**l**\ \|\ **r**][**+v**] ] [|-D|\ *dfile* ]
[ |-E|\ *line* ]
[ |-N| ] 
[ |SYN_OPT-R| ]
[ |-S|\ *method*/*modifiers* ] [ |-T|\ [*radius*\ [**u**]][**+e**\ \|\ **p**]]
[ |-V|\ [*level*] ] [ |-Z| ]
[ |SYN_OPT-b| ] 
[ |SYN_OPT-d| ] 
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ] 
[ |SYN_OPT-g| ] 
[ |SYN_OPT-h| ] 
[ |SYN_OPT-i| ] 
[ |SYN_OPT-j| ] 
[ |SYN_OPT-n| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-s| ]
[ **-:**\ [**i**\ \|\ **o**] ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdtrack** reads one or more grid files (or a Sandwell/Smith IMG
files) and a table (from file or standard input; but see **-E** for
exception) with (x,y) [or (lon,lat)] positions in the first two columns
(more columns may be present). It interpolates the grid(s) at the
positions in the table and writes out the table with the interpolated
values added as (one or more) new columns. Alternatively (**-C**), the
input is considered to be line-segments and we create orthogonal
cross-profiles at each data point or with an equidistant separation and
sample the grid(s) along these profiles. A bicubic [Default], bilinear,
B-spline or nearest-neighbor (see **-n**) interpolation is used,
requiring boundary conditions at the limits of the region (see **-n**;
Default uses "natural" conditions (second partial derivative normal to
edge is zero) unless the grid is automatically recognized as periodic.)

Required Arguments
------------------

.. _-G:

**-G**\ *gridfile*
    *grdfile* is a 2-D binary grid file with the function f(x,y). If the
    specified grid is in Sandwell/Smith Mercator format you must append
    a comma-separated list of arguments that includes a scale to
    multiply the data (usually 1 or 0.1), the mode which stand for the
    following: (0) Img files with no constraint code, returns data at
    all points, (1) Img file with constraints coded, return data at all
    points, (2) Img file with constraints coded, return data only at
    constrained points and NaN elsewhere, and (3) Img file
    with constraints coded, return 1 at constraints and 0 elsewhere, and
    optionally the max latitude in the IMG file [80.738]. You may repeat
    **-G** as many times as you have grids you wish to sample. 
    Alternatively, use **-G+l**\ *list* to pass a list of file names.
    The grids are sampled and results are output in the order given.
    (See GRID FILE FORMAT below.)

Optional Arguments
------------------

*xyfile*
    This is an ASCII (or binary, see **-bi**)
    file where the first 2 columns hold the (x,y) positions where the
    user wants to sample the 2-D data set.

.. _-A:

**-Af**\ \|\ **p**\ \|\ **m**\ \|\ **r**\ \|\ **R**\ [**+l**]
    For track resampling (if **-C** or **-E** are set) we can select how this is to
    be performed. Append **f** to keep original points, but add
    intermediate points if needed [Default], **m** as **f**, but first
    follow meridian (along y) then parallel (along x), **p** as **f**,
    but first follow parallel (along y) then meridian (along x), **r**
    to resample at equidistant locations; input points are not
    necessarily included in the output, and **R** as **r**, but adjust
    given spacing to fit the track length exactly. Finally, append
    **+l** if distances should be measured along rhumb lines
    (loxodromes). Ignored unless **-C** is used.

.. _-C:

**-C**\ *length*\ [**u**]/\ *ds*\ [*/spacing*][**+a**][**l**\ \|\ **r**][**+v**]
    Use input line segments to create an equidistant and (optionally)
    equally-spaced set of crossing profiles along which we sample the
    grid(s) [Default simply samples the grid(s) at the input locations].
    Specify two length scales that control how the sampling is done:
    *length* sets the full length of each cross-profile, while *ds* is
    the sampling spacing along each cross-profile. Optionally, append
    **/**\ *spacing* for an equidistant spacing between cross-profiles
    [Default erects cross-profiles at the input coordinates]. By
    default, all cross-profiles have the same direction (left to right
    as we look in the direction of the input line segment). Append **+a**
    to alternate the direction of cross-profiles, or **v** to enforce
    either a "west-to-east" or "south-to-north" view. By default the entire
    profiles are output.  Choose to only output the left or right halves
    of the profiles by appending **+l** or **+r**, respectively.  Append suitable units
    to *length*; it sets the unit used for *ds* [and *spacing*] (See
    :ref:`Unit_attributes` below). The default unit for geographic grids is meter while
    Cartesian grids implies the user unit.  The output columns will be
    *lon*, *lat*, *dist*, *azimuth*, *z1*, *z2*, ..., *zn* (The *zi* are
    the sampled values for each of the *n* grids)

.. _-D:

**-D**\ *dfile*
    In concert with **-C** we can save the (possibly resampled) original
    lines to the file *dfile* [Default only saves the cross-profiles].
    The columns will be *lon*, *lat*, *dist*, *azimuth*, *z1*, *z2*, ...
    (sampled value for each grid)

.. _-E:

**-E**\ *line*\ [,\ *line*,...][**+a**\ *az*][**+d**][**+i**\ *inc*\ [**u**]][**+l**\ *length*\ [**u**]][**+n**\ *np*][**+o**\ *az*][**+r**\ *radius*\ [**u**]
    Instead of reading input track coordinates, specify profiles via
    coordinates and modifiers. The format of each *line* is
    *start*/*stop*, where *start* or *stop* are either *lon*/*lat* (*x*/*y* for
    Cartesian data) or a 2-character XY key that uses the :doc:`text`-style
    justification format to specify a point on the map as
    [LCR][BMT]. In addition, you can use Z-, Z+ to mean the global
    minimum and maximum locations in the grid (only available if only
    one grid is given). Instead of two coordinates you can specify an
    origin and one of **+a**, **+o**, or **+r**. You may append 
    **+i**\ *inc*\ [**u**] to set the sampling interval; if not given then we default to half the minimum grid interval.
    The **+a** sets the azimuth of a profile of given
    length starting at the given origin, while **+o** centers the profile
    on the origin; both require **+l**. For circular sampling specify
    **+r** to define a circle of given radius centered on the origin;
    this option requires either **+n** or **+i**.  The **+n**\ *np* sets
    the desired number of points, while **+l**\ *length* gives the
    total length of the profile. Append **+d** to output the along-track
    distances after the coordinates.  Note: No track file will be read.
    Also note that only one distance unit can be chosen.  Giving different units
    will result in an error.  If no units are specified we default to
    great circle distances in km (if geographic).  If working with geographic
    data you can prepend - (Flat Earth) or + (Geodesic) to *inc*, *length*, or *radius*
    to change the mode of distance calculation [Great Circle].
    Note: If **-C** is set and *spacing* is given the that sampling scheme
    overrules any modifier in **-E**.

.. _-N:

**-N**
    Do *not* skip points that fall outside the domain of the grid(s)
    [Default only output points within grid domain]. 

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. _-S:

**-S**\ *method*/*modifiers*
    In conjunction with **-C**, compute a single stacked profile from
    all profiles across each segment. Append how stacking should be
    computed: **a** = mean (average), **m** = median, **p** = mode
    (maximum likelihood), **l** = lower, **L** = lower but only consider
    positive values, **u** = upper, **U** = upper but only consider
    negative values [**a**]. The *modifiers* control the output;
    choose one or more among these choices: **+a** : Append stacked
    values to all cross-profiles. **+d** : Append stack deviations to
    all cross-profiles. **+r** : Append data residuals (data - stack) to
    all cross-profiles. **+s**\ [*file*] : Save stacked profile to
    *file* [grdtrack_stacked_profile.txt]. **+c**\ *fact* : Compute
    envelope on stacked profile as ±\ *fact* \*\ *deviation* [2].
    Notes: (1) Deviations depend on *method* and are st.dev (**a**), L1
    scale, i.e., 1.4826 \* median absolute deviation (MAD) (for **m** and **p**), or half-range (upper-lower)/2. (2) The
    stacked profile file contains a leading column plus groups of 4-6 columns, with one
    group for each sampled grid. The leading column holds cross distance,
    while the first four columns in a group hold stacked value, deviation, min
    value, and max value, respectively. If *method* is one of
    **a**\ \|\ **m**\ \|\ **p** then we also write the lower and upper
    confidence bounds (see **+c**). When one or more of **+a**, **+d**,
    and **+r** are used then we also append the stacking results to the end of each
    row, for all cross-profiles. The order is always stacked value
    (**+a**), followed by deviations (**+d**) and finally residuals (**+r**).
    When more than one grid is sampled this sequence of 1-3 columns is
    repeated for each grid.

.. _-T:

**-T**\ [*radius*\ [**u**]][**+e**\ \|\ **p**]
   To be used with normal grid sampling, and limited to a single, non-IMG grid.
   If the nearest node to the input point is NaN, search outwards until we find
   the nearest non-NaN node and report that value instead.  Optionally specify
   a search radius which limits the consideration to points within this distance
   from the input point.  To report the location of the nearest node and its
   distance from the input point, append **+e**. The default unit for geographic
   grid distances is spherical degrees.  Use *radius*\ [**u**] to change the unit
   and give *radius* = 0 if you do not want to limit the radius search.
   To instead replace the input point with the coordinates of the nearest node, append **+p**.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-Z:

**-Z**
    Only write out the sampled z-values [Default writes all columns].

**-:**
    Toggles between (longitude,latitude) and (latitude,longitude)
    input/output. [Default is (longitude,latitude)]. 

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is one more than input]. 
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_distcalc.rst_

.. include:: explain_-n.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-s.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_

.. include:: explain_grd_inout_short.rst_

.. include:: explain_grdresample2.rst_

Hints
-----

If an interpolation point is not on a node of the input grid, then a NaN
at any node in the neighborhood surrounding the point will yield an
interpolated NaN. Bicubic interpolation [default] yields continuous
first derivatives but requires a neighborhood of 4 nodes by 4 nodes.
Bilinear interpolation [**-n**] uses only a 2 by 2 neighborhood, but
yields only zeroth-order continuity. Use bicubic when smoothness is
important. Use bilinear to minimize the propagation of NaNs, or lower
*threshold*.

Examples
--------

To sample the file hawaii_topo.nc along the SEASAT track track_4.xyg
(An ASCII table containing longitude, latitude, and SEASAT-derived
gravity, preceded by one header record):

   ::

    grdtrack track_4.xyg -Ghawaii_topo.nc -h > track_4.xygt

To sample the Sandwell/Smith IMG format file topo.8.2.img (2 minute
predicted bathymetry on a Mercator grid) and the Muller et al age grid
age.3.2.nc along the lon,lat coordinates given in the file
cruise_track.xy, try

   ::

    grdtrack cruise_track.xy -Gtopo.8.2.img,1,1 -Gage.3.2.nc > depths-age.d

To sample the Sandwell/Smith IMG format file grav.18.1.img (1 minute
free-air anomalies on a Mercator grid) along 100-km-long cross-profiles
that are orthogonal to the line segment given in the file track.xy,
erecting cross-profiles every 25 km and sampling the grid every 3 km, try

   ::

    grdtrack track.xy -Ggrav.18.1.img,0.1,1 -C100k/3/25 -Ar > xprofiles.txt

To sample the grid data.nc along a line from the lower left to the upper
right corner, using a grid spacing of 1 km on the geodesic, and output distances as well,
try

   ::

    grdtrack -ELB/RT+i1k+d -Gdata.nc -je > profiles.txt

See Also
--------

:doc:`gmt`,
:doc:`gmtconvert`,
:doc:`text`,
:doc:`sample1d`,
:doc:`surface`
