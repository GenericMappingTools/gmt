.. index:: ! grdinterpolate
.. include:: module_core_purpose.rst_

**************
grdinterpolate
**************

|grdinterpolate_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdinterpolate** *cube* | *grd1 grd2 ...*
|-G|\ *outfile*
[ |-D|\ [**+x**\ *xname*][**+y**\ *yname*][**+z**\ *zname*][**+v**\ *vname*][**+s**\ *scale*][**+o**\ *offset*][**+n**\ *invalid*][**+t**\ *title*][**+r**\ *remark*] ]
[ |-E|\ *table*\|\ *line* ]
[ |-F|\ **l**\|\ **a**\|\ **c**\|\ **n**\ [**+1**\|\ **2**] ]
[ |SYN_OPT-R| ]
[ |-S|\ *x/y*\|\ *pointfile*\ [**+h**\ *header*] ]
[ |-T|\ [*min/max*\ /]\ *inc*\ [**+i**\|\ **n**] \|\ |-T|\ *file*\|\ *list* ]
[ |SYN_OPT-V| ]
[ |-Z|\ [*levels*] ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-q| ]
[ |SYN_OPT-s| ]
[ **-:**\ [**i**\|\ **o**] ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdinterpolate** reads a single 3-D netCDF data cube (or a set of 2-D layers)
and interpolates along the 3rd dimension for one or more output levels.  The data cube must
be organized with one or more layers representing the (common) *x* and *y* dimensions
while the 3rd dimension may represent distance or time; we refer to this
dimension as the *level*.  The output layers may be written as a single 3-D cube
or as a set of 2-D layers.  Alternatively, we interpolate the cube along the level-axis
at one or more arbitrary (*x/y*) coordinates (**-S**), resulting in a data table with one or
more level-series, or we slice the 3-D cube along an arbitrary vertical slice and write that
2-D slice to a grid file (**-E**).

Required Arguments
------------------

*cube*
    Name of a 3-D netCDF data cube to be interpolated. Alternatively, with **-Z**,
    you can specify a set of 2-D grid layers instead.

.. _-G:

**-G**\ *outfile*
    This is the output 3D data cube file.  If **-T** only selects a
    single layer then the data cube collapses to a regular 2-D grid file.
    If *outfile* contains a C-language format statement for a floating
    point number (e.g., layer_%6.6f.grd) then we write a series of 2-D
    grid files which will contain the values for each level [Default is
    a 3-D data cube, unless only a single layer is implied by **-T**].
    Also see **-S** for a similar use to write individual level-series tables.

Optional Arguments
------------------

.. _-D:

.. include:: explain_-D_cap.rst_

.. _-E:

**-E**\ *table*\|\ *line*\ [,\ *line*,...][**+a**\ *az*][**+g**][**+i**\ *inc*][**+l**\ *length*][**+n**\ *np*][**+o**\ *az*][**+p**][**+r**\ *radius*][**+x**]
    Specify a crossectinonal profile via a *file* or from specified *line* coordinates and modifiers.
    If a *file*, it must be contain a single segment with either *lon lat* or *lon lat dist* records.
    These must be equidistant.  Alternatively, the format of each *line* is
    *start*/*stop*, where *start* or *stop* are *lon*/*lat* (*x*/*y* for
    Cartesian data). You may append **+i**\ *inc* to set the sampling interval;
    if not given then we default to half the minimum grid interval.  If your *line* starts and
    ends at the same latitude you can force sampling along the parallel with **+p** [great circle].
    For a *line* along parallels or meridians you can add **+g** to report degrees of longitude or latitude
    instead of great circle distances starting at zero.  Append **+x** to compute distances
    along a loxodrome (rhumbline) instead of great circle. Instead of two coordinates
    you can specify an origin and one of **+a**, **+o**, or **+r**.
    The **+a** sets the azimuth of a profile of given
    length starting at the given origin, while **+o** centers the profile
    on the origin; both require **+l**. For circular sampling specify
    **+r** to define a circle of given radius centered on the origin;
    this option requires either **+n** or **+i**.  The **+n**\ *np* modifier sets
    the desired number of points, while **+l**\ *length* gives the
    total length of the profile.
    Also note that only one distance unit can be chosen.  Giving different units
    will result in an error.  If no units are specified we default to
    great circle distances in km (if geographic).  If working with geographic
    data you can use **-j** to control distance calculation mode [Great Circle].
    Use **-G** to set the output grid file name.

.. _-F:

**-Fl**\|\ **a**\|\ **c**\|\ **n**\ [**+1**\|\ **2**]
    Choose from **l** (Linear), **a** (Akima spline), **c** (natural
    cubic spline), and **n** (no interpolation: nearest point) [Default
    is **-Fa**]. You may change the default interpolant; see
    :term:`GMT_INTERPOLANT` in your :doc:`gmt.conf` file.
    You may optionally evaluate the first or second derivative of the spline
    by appending **+1** or **+2**, respectively.

.. _-R:

.. |Add_-R| replace:: Using the **-R** option
    will select a subsection of the grid. If this subsection exceeds the
    boundaries of the grid, only the common region will be output.
.. include:: explain_-R.rst_

.. _-S:

**-S**\ *x/y*\|\ *pointfile*\ [**+h**\ *header*]
    Rather that compute gridded output, create tile/spatial series through the stacked
    grids at the given point (*x/y*) or the list of points in *pointfile*.  If you need
    a series of points defined by an origin and an end point or similar, you can make
    such a file first with :doc:`project`.  By default we simply sample the cube at
    each level.  Use **-T** to interpolate the series.  The grid level (e.g., depth or time)
    will be appended as the last numerical value in the series records.  Use the optional
    **+h** modifier to append *header* to the trailing text of these input points.
    On output the trailing text will become the segment header for the series that originate
    from each point.  By default, the table output is written to standard output.  Use **-G**
    to specify a file name.  Alternatively, if you wish each series to be written to its own
    data file, let the filename in **-G** have a C-format integer specifier (e.g., %d) and we
    will use the running point number to create unique file names.

.. _-T:

**-T**\ [*min/max*\ /]\ *inc*\ [**+i**\|\ **n**] \|\ |-T|\ *file*\|\ *list*
    Make evenly spaced time-steps from *min* to *max* by *inc* [Default uses input times].
    For details on array creation, see `Generate 1D Array`_.  **Note**: If **-Z** is set
    and no output times are set with **-T** we simply rewrite the grid-produced cube as
    a 3-D data cube file and exit.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
..  include:: explain_-V.rst_

.. _-Z:

**-Z**\ [*levels*]
    Read all 2-D input grids given on the command line and assume they represent
    the layers in a 3-D cube [Default reads a single 3-D data cube]. 
    Optionally, append *levels* and assign these to the cube constructed from the grids.
    The *levels* may be specified the same way as in **-T**.  If not given then we default
    to an integer *levels* array starting at 0.

**-:**
    Toggles between (longitude,latitude) and (latitude,longitude)
    input/output in **-S**\ *table*. [Default is (longitude,latitude)].

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

.. include:: explain_-n.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-q.rst_

.. include:: explain_-s.rst_

.. include:: explain_help.rst_

.. include:: explain_array.rst_

File Order
----------

If you provide a series of 2-D files and thus separately assigning the
level via **-Z**, then you must make sure that the order the grids are given
on the command line matches the levels you provide via **-Z**.  Unless your
files are named in lexical order you must be careful with using wildcards
to list all the grids (e.g., \*.nc).

Time Coordinates
----------------

Time coordinates in netCDF grids, be it the x, y, or z coordinate, will
be recognized as such. The variable's **unit** attribute is parsed to
determine the unit and epoch of the time coordinate in the grid. Values
are then converted to the internal time system specified by
:term:`TIME_UNIT` and :term:`TIME_EPOCH` in the
:doc:`gmt.conf` file or on the
command line. The default output is relative time in that time system,
or absolute time when using the option **-f0T**, **-f1T**, or **-f2T**
for x, y, or z coordinate, respectively.

Series creation
---------------

The (optional) table-reading and table-producing **-S** option may require some
of the standard common options associated with table i/o, such as **-b**, **-i**,
**o**, etc., thus these options are available to **grdinterpolate** as well.
Because the coordinates given via **-S** are *not* required to equal the coordinates
of the grid nodes, we are resampling each 2-D layer at the given points via
:doc:`grdtrack`, hence the availability of the **-n** option.

Examples
--------

To extract a single, new 2-D layer from the temperature.nc 3-D cube for level 3400
using a cubic spline, try::

    gmt grdinterpolate temperature.nc -T3400 -Fc -Gtemp_3400.nc

To extract a single, new 2-D layer from the 3-D cube implied by the individual grids
layers_*.nc, with individual layer values given via z.txt, for level 3400
using a linear spline, try::

    gmt grdinterpolate layers_*.nc -Zz.txt -T3400 -Fl -Gtemp_3400.nc

To resample the the temperature.nc 3-D cube for all levels from
1500 to 2500 in steps of 50, using an Akima spline, try::

    gmt grdinterpolate temperature.nc -T1500/2500/50 -Gtemperature_1500_2500.nc -Fa

The same, but this time write individual 2-D grids per layer, try::

    gmt grdinterpolate temperature.nc -T1500/2500/50 -Gtemperature_%4.0f.nc -Fa

To extract a time-series through the grids deformation_*.nc at the location (115W, 33N),
with the times of each grid provided by the file dates.txt, and append the string
"Some like it hot" to the segment header for the series, try::

    gmt grdinterpolate deformation_*.nc -Zdates.txt -S115W/33N+h"Some like it hot" > record.txt

To extract a vertical slice of the 3-D grid S362ANI_kmps.nc with seismic velocities that goes
through the Hawaii hotspot, selecting cube vs (Isotropic Shear Velocity) and letting the
distances be longitude degrees along the parallel, try::

    gmt grdinterpolate S362ANI_kmps.nc?vs -E180/20/220/20+i1d+g+p -T25/500/25 -Gslice.nc

See Also
--------

:doc:`gmt.conf`,
:doc:`gmt`,
:doc:`grdedit`,
:doc:`grdcut`,
:doc:`project`
