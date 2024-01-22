.. index:: ! sample1d
.. include:: module_core_purpose.rst_

********
sample1d
********

|sample1d_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt sample1d** [ *table* ]
[ |-A|\ [**f**\|\ **p**\|\ **m**\|\ **r**\|\ **R**][**+d**][**+l**] ]
[ |-C|\ [*section*/]\ *master*\|\ *cpt*\|\ *color*\ :math:`_1`,\ *color*\ :math:`_2`\ [,\ *color*\ :math:`_3`\ ,...]\ [**+h**\ [*hinge*]][**+i**\ *dz*][**+u**\|\ **U**\ *unit*][**+s**\ *fname*] ]
[ |-E| ]
[ |-F|\ **a**\|\ **c**\|\ **e**\|\ **l**\|\ **n**\|\ **s**\ *p*\ [**+d1**\|\ **2**] ]
[ |-N|\ *col* ]
[ |-T|\ [*min/max*\ /]\ *inc*\ [**+a**][**+i**\|\ **n**][**+u**] \| [|-T|\ *file*\|\ *list*]]
[ |SYN_OPT-V| ]
[ |-W|\ *col* ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-j| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-q| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**sample1d** reads a multi-column ASCII [or binary] data set from file
[or standard input] and interpolates the time-series or spatial profile at locations
where the user needs the values. The user must provide the column number
of the independent (monotonically increasing **or** decreasing)
variable, here called *time* (it may of course be any type of quantity) when that is not the first column in data set.
Equidistant or arbitrary sampling can be selected. All columns
are resampled based on the new sampling interval. Several interpolation
schemes are available, in addition to a *smoothing* spline which trades off misfit
for curvature. Extrapolation outside the range of the input data
is not supported. By giving a CPT via |-C| we will add *r*, *g*, *b*, *a* columns to the output
based on the last input data column.

Required Arguments
------------------

*table*
    This is one or more ASCII [of binary, see
    **-bi**] files with one column containing the
    independent *time* variable (which must be monotonically in/de-creasing)
    and any number of optional columns holding other data values. If no file is
    provided, **sample1d** reads from standard input.

Optional Arguments
------------------

.. _-A:

**-A**\ [**f**\|\ **p**\|\ **m**\|\ **r**\|\ **R**][**+d**][**+l**]
    For track resampling (if |-T|...\ *unit* is set) we can select how
    this is to be performed via these directives:

    - **f** - Keep original points, but add intermediate points if needed;
      note this selection does not necessarily yield equidistant points [Default].
    - **m** - Same as **f**, but first follow meridian (along *y*) then parallel (along *x*).
    - **p** - Same as **f**, but first follow parallel (along *x*) then meridian (along *y*).
    - **r** - Resample at equidistant locations; input points are not
      necessarily included in the output.
    - **R** - Same as **r**, but adjust given spacing to fit the track length exactly.

    A few modifiers can also be used:

    - **+d** - Delete duplicate input records (identified by having
      no change in the time column, and
    - **+l** - If distances should be measured along rhumb lines (loxodromes).
    
    **Note**: Calculation mode for loxodromes is spherical, hence **-je**
    cannot be used in combination with **+l**.

.. _-C:

.. include:: dump_rgb.rst_

.. _-E:

**-E**
    If the input dataset contains records with trailing text then we will attempt
    to add these to output records that exactly match the input times.  Output records
    that have no matching input record times will have no trailing text appended [Default
    ignores trailing text].

.. _-F:

.. include:: explain_interpolant.rst_

.. figure:: /_images/GMT_splines.*
   :width: 500 px
   :align: center

   The |-F| option lets you choose among several interpolants, including
   one that is approximate (the smoothing spline).  You can also specify
   that you actually need a derivative of the solution instead of the value.

.. _-N:

**-N**\ *col*
    Sets the column number of the independent *time* variable [Default is 0
    (first)].

.. _-T:

**-T**\ [*min/max*\ /]\ *inc*\ [**+a**][**+i**\|\ **n**][**+u**] \| [**-T**\ *file*\|\ *list*]
    Make evenly spaced time-steps from *min* to *max* by *inc* [Default uses input times].
    The form **-T**\ *list* means a online list of *time* coordinates like for example: **-T**\ *13,15,16,22.5*.
    For details on array creation, see `Generate 1-D Array`_.
    If |-A| is not set, it defaults to |-A|\ **f**. **Note**: For resampling of spatial
    (*x, y*) or (*lon, lat*) series you must give an increment with a valid distance unit;
    see `Units`_ for map units or use **c** if plain Cartesian coordinates. The first two
    columns must contain the spatial coordinates. From these we calculate distances in the
    chosen units and interpolate using this parametric series.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *col*
    Sets the column number of the weights to be used with a smoothing cubic
    spline.  Requires **-Fs**.

.. |Add_-bi| replace:: [Default is 2 (or at least the number of columns implied by |-T|)].
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

.. include:: explain_distcalc.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-q.rst_

.. include:: explain_-w.rst_

.. include:: explain_-s.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_precision.rst_

.. include:: explain_array.rst_

Notes
-----

The smoothing spline *s(t)* requires a fit parameter *p* that allows for the trade-off between an
exact interpolation (fitting the data exactly; large *p*) to minimizing curvature (*p* approaching 0).
Specifically, we seek to minimize

.. math::

    F_p (s)= K (s) + p E (s), \quad p > 0,

where the misfit is evaluated as

.. math::

    E (s)= \sum^n_{i=1} \left [ \frac{s(t_i) - y_i}{\sigma_i} \right ]^2

and the curvature is given by the integral over the domain of the second derivative of the spline

.. math::

    K (s) = \int ^b _a [s''(t) ]^2 dt.


Trial and error may be needed to select a suitable *p*.

Examples
--------

.. include:: explain_example.rst_

To resample the file profiles.tdgmb, which contains
(time,distance,gravity,magnetics,bathymetry) records, at 1 km equidistant
intervals using Akima's spline, use::

    gmt sample1d profiles.tdgmb -N1 -Fa -T1 > profiles_equi_d.tdgmb

To resample the file depths.txt at positions listed in the file
grav_pos.dg, using a cubic spline for the interpolation, use::

    gmt sample1d depths.txt -Tgrav_pos.dg -Fc > new_depths.txt

To resample the file points.txt every 0.01 from 0-6, using a cubic spline for the
interpolation, but output the first derivative instead (the slope), try::

    gmt sample1d points.txt -T0/6/0.01 -Fc+d1 > slopes.txt

To resample the file track.txt which contains *lon, lat, depth* every 2
nautical miles, use::

    gmt sample1d track.txt -T2n -AR > new_track.txt

To do approximately the same, but make sure the original points are
included, use::

    gmt sample1d track.txt -T2n -Af > new_track.txt

To obtain a rhumb line (loxodrome) sampled every 5 km instead, use::

    gmt sample1d track.txt -T5k -AR+l > new_track.txt

To sample temperatures.txt every month from 2000 to 2018, use::

    gmt sample1d temperatures.txt -T2000T/2018T/1o > monthly_temp.txt

To use a smoothing spline on a topographic profile for a given fit parameter, try::

    gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs0.001 > smooth.txt

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`greenspline`,
:doc:`filter1d`
