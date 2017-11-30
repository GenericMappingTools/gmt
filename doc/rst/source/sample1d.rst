.. index:: ! sample1d

********
sample1d
********

.. only:: not man

    sample1d - Resample 1-D table data using splines

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**sample1d** [ *table* ]
[ |-A|\ **f**\ \|\ **p**\ \|\ **m**\ \|\ **r**\ \|\ **R**\ [**+l**] ]
[ |-F|\ **l**\ \|\ **a**\ \|\ **c**\ \|\ **n**\ [**+1**\ \|\ **+2**] ]
[ |-I|\ *inc*\ [*unit*] ]
[ |-N|\ *knotfile* ]
[ |-S|\ *start*\ [/*stop*] ]
[ |-T|\ *col* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]

|No-spaces|

Description
-----------

**sample1d** reads a multi-column ASCII [or binary] data set from file
[or standard input] and interpolates the time-series or spatial profile at locations
where the user needs the values. The user must provide the column number
of the independent (monotonically increasing **or** decreasing)
variable, here called *time* (it may of course be any type of quantity).
Equidistant or arbitrary sampling can be selected. All columns
are resampled based on the new sampling interval. Several interpolation
schemes are available. Extrapolation outside the range of the input data
is not supported. 

Required Arguments
------------------

None.

Optional Arguments
------------------

*table*
    This is one or more ASCII [of binary, see
    **-bi**] files with one column containing the
    independent *time* variable (which must be monotonically in/de-creasing)
    and the remaining columns holding other data values. If no file is
    provided, **sample1d** reads from standard input.

.. _-A:

**-A**\ **f**\ \|\ **p**\ \|\ **m**\ \|\ **r**\ \|\ **R**
    For track resampling (if **-T**...\ *unit* is set) we can select how
    this is to be performed. Append **f** to keep original points, but
    add intermediate points if needed; note this selection does not
    necessarily yield equidistant points [Default], **m** as **f**, but
    first follow meridian (along y) then parallel (along x), **p** as
    **f**, but first follow parallel (along y) then meridian (along x),
    **r** to resample at equidistant locations; input points are not
    necessarily included in the output, and **R** as **r**, but adjust
    given spacing to fit the track length exactly. Finally, append
    **+l** if distances should be measured along rhumb lines (loxodromes).

.. _-F:

**-Fl**\ \|\ **a**\ \|\ **c**\ \|\ **n**\ [**+1**\ \|\ **+2**]
    Choose from **l** (Linear), **a** (Akima spline), **c** (natural
    cubic spline), and **n** (no interpolation: nearest point) [Default
    is **-Fa**]. You may change the default interpolant; see
    :ref:`GMT_INTERPOLANT <GMT_INTERPOLANT>` in your :doc:`gmt.conf` file.
    You may optionally evaluate the first or second derivative of the spline
    by appending **1** or **2**, respectively.

.. _-I:

**-I**\ *inc*\ [*unit*]
    *inc* defines the sampling interval [Default is the separation
    between the first and second abscissa point in the *table*]. Append
    a distance unit (see UNITS) to indicate that the first two columns
    contain longitude, latitude and you wish to resample this path with
    a spacing of *inc* in the chosen units. For sampling of (x, y)
    Cartesian tracks, specify the unit as c. Use **-A** to control how
    path resampling is performed.

.. _-N:

**-N**\ *knotfile*
    *knotfile* is an optional ASCII file with the *time* locations where the
    data set will be resampled in the first column. Note: If **-H** is
    selected it applies to both *table* and *knotfile*. Also note that
    **-i** never applies to *knotfile* since we always consider the
    first column only.

.. _-S:

**-S**\ *start*\ [*stop*] 
    For equidistant sampling, *start* indicates the *time* of the
    first output value. [Default is the smallest even multiple of *inc*
    inside the range of *table*]. Optionally, append /*stop* to
    indicate the *time* of the last output value [Default is the
    largest even multiple of *inc* inside the range of *table*].

.. _-T:

**-T**\ *col*
    Sets the column number of the independent *time* variable [Default is 0
    (first)]. 

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 (or at least the number of columns implied by **-T**)]. 
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
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

.. include:: explain_-ocols.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_

Calendar Time Sampling
----------------------

If the abscissa are calendar times then you must use the **-f** option
to indicate this. Furthermore, **-I** then expects an increment in the
current :ref:`TIME_UNIT <TIME_UNIT>` units. There is not yet support for variable
intervals such as months.

Examples
--------

To resample the file profiles.tdgmb, which contains
(time,distance,gravity,magnetics,bathymetry) records, at 1km equidistant
intervals using Akima's spline, use

   ::

    gmt sample1d profiles.tdgmb -I1 -Fa -T1 > profiles_equi_d.tdgmb

To resample the file depths.dt at positions listed in the file
grav_pos.dg, using a cubic spline for the interpolation, use

   ::

    gmt sample1d depths.dt -Ngrav_pos.dg -Fc > new_depths.dt

To resample the file points.txt every 0.01 from 0-6, using a cubic spline for the
interpolation, but output the first derivative instead (the slope), try

   ::

    gmt sample1d points.txt S0/6 -I0.01 -Fc+1 > slopes.txt

To resample the file track.txt which contains lon, lat, depth every 2
nautical miles, use

   ::

    gmt sample1d track.txt -I2n -AR > new_track.dt

To do approximately the same, but make sure the original points are
included, use

   ::

    gmt sample1d track.txt -I2n -Af > new_track.dt

To obtain a rhumb line (loxodrome) sampled every 5 km instead, use

   ::

    gmt sample1d track.txt -I5k -AR+l > new_track.dt

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`greenspline`,
:doc:`filter1d`
