.. index:: ! grdtrack
.. include:: module_core_purpose.rst_

********
grdtrack
********

|grdtrack_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdtrack** [ *table* ] |-G|\ *grd1* [ |-G|\ *grd2* ... ]
[ |-A|\ [**f**\|\ **p**\|\ **m**\|\ **r**\|\ **R**][**+l**] ]
[ |-C|\ *length*/\ *ds*\ [*/spacing*][**+a**\|\ **v**][**d**\|\ **f**\ *value*][**l**\|\ **r**] ]
[ |-D|\ *dfile* ]
[ |-E|\ *line* ]
[ |-F|\ [**+b**][**+n**][**+r**][**+z**\ *z0*] ]
[ |-N| ]
[ |SYN_OPT-R| ]
[ |-S|\ *method*/*modifiers* ]
[ |-S|\ [**a**\|\ **l**\|\ **L**\|\ **m**\|\ **p**\|\ **u**\|\ **U**][**+a**][**+c**][**+d**][**+r**][**+s**\ [*file*] ]
[ |-T|\ [*radius*][**+e**\|\ **p**]]
[ |-V|\ [*level*] ]
[ |-Z| ]
[ |SYN_OPT-a| ]
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
[ |SYN_OPT-q| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-w| ]
[ **-:**\ [**i**\|\ **o**] ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdtrack** reads one or more grid files (or a Sandwell/Smith IMG
files) and a table (from file or standard input; but see |-E| for
exception) with (*x, y*) [or (*lon, lat*)] positions in the first two columns
(more columns may be present). It interpolates the grid(s) at the
positions in the table and writes out the table with the interpolated
values added as (one or more) new columns. Alternatively (|-C|), the
input is considered to be line-segments and we create orthogonal
cross-profiles at each data point or with an equidistant separation and
sample the grid(s) along these profiles. A bicubic [Default], bilinear,
B-spline or nearest-neighbor (see **-n**) interpolation is used,
requiring boundary conditions at the limits of the region (see **-n**;
Default uses "natural" conditions (second partial derivative normal to
edge is zero) unless the grid is automatically recognized as periodic.)

Required Arguments
------------------

*table*
    This is an ASCII (or binary, see **-bi**) file where the first 2 columns
    hold the (*x, y*) positions where the user wants to sample the 2-D data set.
    If no tables are given then we read from standard input. If |-E| is set
    then no input table is read since we will create one from the given
    |-E| parameters.

.. _-G:

**-G**\ *gridfile*
    *gridfile* is a 2-D binary grid file with the function *f*\ (*x, y*). If the
    specified grid is in Sandwell/Smith Mercator format you must append
    a comma-separated list of arguments that includes a scale to
    multiply the data (usually 1 or 0.1), the mode which stand for the
    following: (0) Img files with no constraint code, returns data at
    all points, (1) Img file with constraints coded, return data at all
    points, (2) Img file with constraints coded, return data only at
    constrained points and NaN elsewhere, and (3) Img file
    with constraints coded, return 1 at constraints and 0 elsewhere, and
    optionally the max latitude in the IMG file [80.738]. You may repeat
    |-G| as many times as you have grids you wish to sample.
    Alternatively, use **-G+l**\ *list* to pass a file whose first word
    in the trailing text record will be extracted as the file names.
    Note, this means that file must have at least one numeric column before the text holding the grid names.
    The grids are sampled and results are output in the order given.
    (See :ref:`Grid File Formats <grd_inout_full>`). **Note**: If *gridfile*
    is a remote global grid and no registration is specified then **grdtrack**
    will default to gridline registration (instead of the default pixel registration)
    to ensure all input points are inside the grid.

Optional Arguments
------------------

.. _-A:

**-A**\ [**f**\|\ **p**\|\ **m**\|\ **r**\|\ **R**][**+l**]
    For track resampling (if |-C| or |-E| are set) we can select how this is to
    be performed. Append **f** to keep original points, but add
    intermediate points if needed [Default], **m** as **f**, but first
    follow meridian (along y) then parallel (along x), **p** as **f**,
    but first follow parallel (along y) then meridian (along x), **r**
    to resample at equidistant locations; input points are not
    necessarily included in the output, and **R** as **r**, but adjust
    given spacing to fit the track length exactly. Finally, append
    **+l** if geographic distances should be measured along rhumb lines
    (loxodromes) instead of great circles. Ignored unless |-C| is used.

.. _-C:

**-C**\ *length*/\ *ds*\ [*/spacing*][**+a**\|\ **v**][**d**\|\ **f**\ *value*][**l**\|\ **r**]
    Use input line segments to create an equidistant and (optionally)
    equally-spaced set of crossing profiles along which we sample the
    grid(s).
    Specify two length scales that control how the sampling is done:
    *length* sets the full length of each cross-profile, while *ds* is
    the sampling spacing along each cross-profile. Optionally, append
    **/**\ *spacing* for an equidistant spacing between cross-profiles
    [Default erects cross-profiles at the input coordinates]; see |-A|
    for how resampling the input track is controlled. By
    default, all cross-profiles have the same direction (left to right
    as we look in the direction of the input line segment). Append **+a**
    to alternate the direction of cross-profiles, or **+v** to enforce
    either a "west-to-east" or "south-to-north" view. By default the entire
    profiles are output.  Choose to only output the left or right halves
    of the profiles by appending **+l** or **+r**, respectively.  Append suitable units
    to *length*; it sets the unit used for *ds* [and *spacing*] (See
    `Units`_ below). The default unit for geographic grids is meter while
    Cartesian grids implies the user unit.  The output columns will be
    *lon*, *lat*, *dist*, *azimuth*, *z1*, *z2*, ..., *zn* (The *zi* are
    the sampled values for each of the *n* grids). Use **+d** to
    change the profiles from being orthogonal to the line by the given
    *deviation* [0]. Looking in the direction of the line, a positive *deviation*
    will rotate the crosslines clockwise and a negative one will rotate them
    counter-clockwise.  Finally, you can use **+f** to set a fixed azimuth
    for all profiles. **Note**: If |-C| is set and *spacing* is given then
    that sampling scheme overrules any modifier set in |-E|.
    Currently, there is a bug when |-C| and |-E| use different units 
    (see PR `#8728 <https://github.com/GenericMappingTools/gmt/pull/8728>`_ ).
    If you use both, please manually specify the same unit for each.

.. _-D:

**-D**\ *dfile*
    In concert with |-C| we can save the (possibly resampled) original
    lines to the file *dfile* [Default only saves the cross-profiles].
    The columns will be *lon*, *lat*, *dist*, *azimuth*, *z1*, *z2*, ...
    (sampled value for each grid).

.. _-E:

.. include:: explain_lines.rst_

.. _-F:

**-F**\ [**+b**][**+n**][**+r**][**+z**\ *z0*]
    Find critical points along each cross-profile as a function of along-track distance.
    Requires |-C| and a single input grid (*z*). We examine each cross-profile generated
    and report (*dist*, *lonc*, *latc*, *distc*, *azimuthc*, *zc*) at the center peak of
    maximum *z* value, (*lonl*, *latl*, *distl*) and (*lonr*, *latr*, *distr*)
    at the first and last non-NaN point whose *z*-value exceeds *z0*, respectively,
    and the *width* based on the two extreme points found. Here, *dist* is the distance
    along the original input *trackfile* and the other 12 output columns are a function
    of that distance.  When searching for the center peak and the extreme first and last
    values that exceed the threshold we assume the profile is positive up.  If we instead
    are looking for a trough then you must use **+n** to temporarily flip the profile to
    positive (internally). The threshold *z0* value is always given as >= 0; use **+z** to change it [0].
    Alternatively, use **+b** to determine the balance point and standard deviation of the
    profile; this is the weighted mean and weighted standard deviation of the distances,
    with *z* acting as the weight. Finally, use **+r** to obtain the weighted rms about the
    cross-track center (*distc* == 0).  **Note**: We round the exact results to the nearest
    distance nodes along the cross-profiles.  We write 13 output columns per track:
    *dist, lonc, latc, distc, azimuthc, zc, lonl, latl, distl, lonr, latr, distr, width*.

.. _-N:

**-N**
    Do *not* skip points that fall outside the domain of the grid(s)
    [Default only output points within grid domain].

.. |Add_-R| replace:: |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-S:

**-S**\ [**a**\|\ **l**\|\ **L**\|\ **m**\|\ **p**\|\ **u**\|\ **U**][**+a**][**+c**][**+d**][**+r**][**+s**\ [*file*]
    In conjunction with |-C|, compute a single stacked profile from
    all profiles across each segment. Append a method for how stacking should be
    computed: **a** = mean (average), **m** = median, **p** = mode
    (maximum likelihood), **l** = lower, **L** = lower but only consider
    positive values, **u** = upper, **U** = upper but only consider
    negative values [**a**]. The *modifiers* control the output;
    choose one or more among these choices: **+a** : Append stacked
    values to all cross-profiles. **+d** : Append stack deviations to
    all cross-profiles. **+r** : Append data residuals (data - stack) to
    all cross-profiles. **+s**\ [*file*] : Save stacked profile to
    *file* [stacked_profile.txt]. **+c**\ *fact* : Compute
    uncertainty envelope on stacked profile as ±\ *fact* \*\ *deviation* [2].
    **Notes**: (1) Deviations depend on *method* and are standard deviation (**a**), L1
    scale, i.e., 1.4826 \* median absolute deviation (MAD) (for **m** and **p**),
    or half-range (upper-lower)/2 (for **l**, **L**, **u** and **U**). (2) The
    stacked profile file contains a leading column plus groups of 4-6 columns, with one
    group for each sampled grid. The leading column holds cross distance,
    while the first four columns in a group hold stacked value, deviation, min
    value, and max value, respectively. If *method* is one of
    **a**\|\ **m**\|\ **p** then we also write two additional columns: the lower and upper
    confidence bounds (see **+c**). When one or more of **+a**, **+d**,
    and **+r** are used then we also append the stacking results to the end of each
    row, for all cross-profiles. The order is always stacked value
    (**+a**), followed by deviations (**+d**) and finally residuals (**+r**);
    actual output depends on which of these modifiers were actually used.
    When more than one grid is sampled this sequence of 1-3 columns is
    repeated for each grid. (3) See Illustration Gallery 33 for an example
    of grid profile stacking.

.. _-T:

**-T**\ [*radius*][**+e**\|\ **p**]
   To be used with normal grid sampling, and limited to a single, non-IMG grid.
   If the nearest node to the input point is NaN, search outwards until we find
   the nearest non-NaN node and report that value instead.  Optionally specify
   a search radius which limits the consideration to points within this distance
   from the input point.  To report the location of the nearest node and its
   distance from the input point, append **+e**. The default unit for geographic
   grid distances is spherical degrees.  Use *radius* to change the unit
   and give *radius* = 0 if you do not want to limit the radius search.
   To instead replace the input point with the coordinates of the nearest node, append **+p**.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**
    Only write out the sampled z-values [Default writes all columns].
    **Note**: If used in conjunction with **-s** then the default
    column becomes 0 instead of 2.  If specifying specific columns
    in **-s** then start numbering the z-columns from 0 instead of 2.

.. include:: explain_-aspatial.rst_

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

.. include:: explain_-q.rst_

.. include:: explain_-s.rst_

.. include:: explain_-w.rst_

**-:**
    Toggles between (longitude,latitude) and (latitude,longitude)
    input/output. [Default is (longitude,latitude)].

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_

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

.. include:: explain_example.rst_

To extract a profile along a great circle between (0,0) to (20,20) from the remote grid earth_relief_05m,
and only write out (dist, topo) records, try::

    gmt grdtrack -G@earth_relief_05m -R0/20/0/20 -EBL/TR+d -o3,2 > profile.txt

To sample the file hawaii_topo.nc along the SEASAT track track_4.xyg
(An ASCII table containing longitude, latitude, and SEASAT-derived
gravity, preceded by one header record)::

    gmt grdtrack track_4.xyg -Ghawaii_topo.nc -h > track_4.xygt

To sample the Sandwell/Smith IMG format file topo.8.2.img (2 minute
predicted bathymetry on a Mercator grid) and the Muller et al age grid
age.3.2.nc along the (*lon, lat*) coordinates given in the file
cruise_track.xy, try::

    gmt grdtrack cruise_track.xy -Gtopo.8.2.img,1,1 -Gage.3.2.nc > depths-age.txt

To sample the Sandwell/Smith IMG format file grav.18.1.img (1 minute
free-air anomalies on a Mercator grid) along 100-km-long cross-profiles
that are orthogonal to the line segment given in the file track.xy,
erecting cross-profiles every 25 km and sampling the grid every 3 km, try

::

  gmt grdtrack track.xy -Ggrav.18.1.img,0.1,1 -C100k/3/25 -Ar > xprofiles.txt

The same thing, but now determining the central anomaly location along track,
with a threshold of 25 mGal, try::

    gmt grdtrack track.xy -Ggrav.18.1.img,0.1,1 -C100k/3/25 -F+z25 > locations.txt

To sample the grid data.nc along a line from the lower left to the upper
right corner, using a grid spacing of 1 km on the geodesic, and output distances as well,
try::

    gmt grdtrack -ELB/RT+i1k+d -Gdata.nc -je > profiles.txt

See Also
--------

:doc:`gmt`,
:doc:`gmtconvert`,
:doc:`text`,
:doc:`sample1d`,
:doc:`surface`
