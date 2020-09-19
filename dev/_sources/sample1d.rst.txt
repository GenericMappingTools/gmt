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
[ |-A|\ **f**\|\ **p**\|\ **m**\|\ **r**\|\ **R**\ [**+d**][**+l**] ]
[ |-F|\ **l**\|\ **a**\|\ **c**\|\ **n**\|\ **s**\ *p*\ [**+d1**\|\ **2**] ]
[ |-N|\ *col* ]
[ |-T|\ [*min/max*\ /]\ *inc*\ [**+a**\|\ **n**] \|\ |-T|\ *file*\|\ *list* ]
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

**-A**\ **f**\|\ **p**\|\ **m**\|\ **r**\|\ **R**\ [**+d**][**+l**]
    For track resampling (if **-T**...\ *unit* is set) we can select how
    this is to be performed. Append **f** to keep original points, but
    add intermediate points if needed; note this selection does not
    necessarily yield equidistant points [Default], **m** as **f**, but
    first follow meridian (along y) then parallel (along x), **p** as
    **f**, but first follow parallel (along y) then meridian (along x),
    **r** to resample at equidistant locations; input points are not
    necessarily included in the output, and **R** as **r**, but adjust
    given spacing to fit the track length exactly. Finally, append
    **+d** to delete duplicate input records (identified by having
    no change in the time column, and
    **+l** if distances should be measured along rhumb lines (loxodromes).

.. _-F:

**-Fl**\|\ **a**\|\ **c**\|\ **n**\ **s**\ *p*\ [**+d1**\|\ **2**]
    Choose from **l** (Linear), **a** (Akima spline), **c** (natural
    cubic spline), **n** (no interpolation: nearest point), or **s**
    (smoothing cubic spline; append fit parameter *p*) [Default
    is **-Fa**]. You may change the default interpolant; see
    :term:`GMT_INTERPOLANT` in your :doc:`gmt.conf` file.
    You may optionally evaluate the first or second derivative of the spline
    by appending **+d1** or **+d2**, respectively.

.. _-N:

**-N**\ *col*
    Sets the column number of the independent *time* variable [Default is 0
    (first)].

.. _-T:

**-T**\ [*min/max*\ /]\ *inc*\ [**+a**\|\ **n**] \|\ |-T|\ *file*\|\ *list*
    Make evenly spaced time-steps from *min* to *max* by *inc* [Default uses input times].
    For details on array creation, see `Generate 1D Array`_.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *col*
    Sets the column number of the weights to be used with a smoothing cubic
    spline.  Requires **-Fs**.

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

.. include:: explain_-q.rst_

.. include:: explain_distcalc.rst_

.. include:: explain_-ocols.rst_

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
(time,distance,gravity,magnetics,bathymetry) records, at 1km equidistant
intervals using Akima's spline, use

   ::

    gmt sample1d profiles.tdgmb -N1 -Fa -T1 > profiles_equi_d.tdgmb

To resample the file depths.dt at positions listed in the file
grav_pos.dg, using a cubic spline for the interpolation, use

   ::

    gmt sample1d depths.txt -Tgrav_pos.dg -Fc > new_depths.txt

To resample the file points.txt every 0.01 from 0-6, using a cubic spline for the
interpolation, but output the first derivative instead (the slope), try

   ::

    gmt sample1d points.txt -T0/6/0.01 -Fc+d1 > slopes.txt

To resample the file track.txt which contains lon, lat, depth every 2
nautical miles, use

   ::

    gmt sample1d track.txt -T2n -AR > new_track.txt

To do approximately the same, but make sure the original points are
included, use

   ::

    gmt sample1d track.txt -T2n -Af > new_track.txt

To obtain a rhumb line (loxodrome) sampled every 5 km instead, use

   ::

    gmt sample1d track.txt -T5k -AR+l > new_track.txt

To sample temperatures.txt every month from 2000 to 2018, use

   ::

    gmt sample1d temperatures.txt -T2000T/2018T/1o > monthly_temp.txt

To use a smoothing spline on a topographic profile for a given fit parameter, try

   ::

    gmt sample1d @topo_crossection.txt -T300/500/0.1 -Fs0.001 > smooth.txt

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`greenspline`,
:doc:`filter1d`
