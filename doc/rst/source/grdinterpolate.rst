.. index:: ! grdinterpolate
.. include:: module_core_purpose.rst_

**************
grdinterpolate
**************

|grdinterpolate_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdinterpolate** *3Dgrid* | *grd1 grd2 ...*
|-G|\ *outgrid*
**-T**\ [*min/max*\ /]\ *inc*\ [**+n**] \|\ |-T|\ *file*\|\ *list*
[ |-F|\ **l**\|\ **a**\|\ **c**\|\ **n**\ [**+1**\|\ **2**] ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |-Z|\ **i**\ *levels*\|\ **o** ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdinterpolate** reads a single 3-D netCDF data cube (or a set of 2-D layers)
and interpolates along the 3rd dimension for one or more output levels.  The data cube must
be organized with one or more layers representing the *x* and *y* dimensions
while the 3rd dimension may represent distance or time; we refer to this
dimension as the *level*.  The output layers may be written as a single 3-D cube
or as a set of 2-D layers.

Required Arguments
------------------

*3Dgrid*
    Name of a 3-D netCDF data cube to be interpolated. Alternatively, with **-Zi**,
    you can specify a set of 2-D grid layers instead.

.. _-G:

**-G**\ *outgrid*
    This is the output 3D data cube file.  If **-T** only selects a
    single layer then the data cube collapses to a regular 2-D grid file.
    If **-Zo** is used then *outgrid* must contain a C-format statement
    for a floating point number.

.. _-T:

**-T**\ [*min/max*\ /]\ *inc*\ [**+n**] \|\ |-T|\ *file*\|\ *list*
    Make evenly spaced time-steps from *min* to *max* by *inc* [Default uses input times].
    For details on array creation, see `Generate 1D Array`_.

Optional Arguments
------------------

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

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
..  include:: explain_-V.rst_

.. _-Z:

**-Zi**\ *levels*\|\ **o**
    Use **-Zi** to obtain the *levels* and then we read the corresponding number of
    2-D input grids given on the command line [Default is a single 3-D data cube].
    The *levels* are specified the same way as in **-T**.
    Use **-Zo** to write the 3-D data cube as a series of 2-D grids instead.  If used,
    then the *outgrid* name given by **-G** must contain a C-language format statement
    for a floating point number (for instance, one can try layer_%6.6f.grd) which will contain the level
    for each grid [Default is a 3-D data cube, unless only one layer is implied by **-T**].

.. include:: explain_help.rst_

.. include:: explain_array.rst_

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

Examples
--------

To extract a single, new 2-D layer from the temperature.nc 3-D cube for level 3400
using a cubic spline, try

   ::

    gmt grdinterpolate temperature.nc -T3400 -Fc -Gtemp_3400.nc

To extract a single, new 2-D layer from the 3-D cube implied by the individual grids
layers_*.nc, with individual layer values given via z.txt, for level 3400
using a linear spline, try

   ::

    gmt grdinterpolate layers_*.nc -Ziz,txt -T3400 -Fl -Gtemp_3400.nc

To resample the the temperature.nc 3-D cube for all levels from
1500 to 2500 in steps of 50, using an Akima spline, try

   ::

    gmt grdinterpolate temperature.nc -T1500/2500/50 -Gtemperature_1500_2500.nc -Fa

The same, but this time write individual 2-D grids per layer:

   ::

    gmt grdinterpolate temperature.nc -T1500/2500/50 -Gtemperature_%4.0f.nc -Fa -Zo

See Also
--------

:doc:`gmt.conf`,
:doc:`gmt`,
:doc:`grdedit`,
:doc:`grdcut`
