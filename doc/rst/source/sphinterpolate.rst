.. index:: ! sphinterpolate
.. include:: module_core_purpose.rst_

**************
sphinterpolate
**************

|sphinterpolate_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt sphinterpolate** [ *table* ]
|-G|\ *grdfile*
|SYN_OPT-I|
|SYN_OPT-R|
[ |-D|\ [*east*] ]
[ |-Q|\ *mode*\ [*options*] ]
[ |SYN_OPT-V| ]
[ |-Z| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**sphinterpolate** reads one or more ASCII [or binary] files (or
standard input) containing *lon, lat, z* and performs a Delaunay
triangulation to set up a spherical interpolation in tension. The final
grid is saved to the specified file. Several options may be used to
affect the outcome, such as choosing local versus global gradient
estimation or optimize the tension selection to satisfy one of four
criteria.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-G:

**-G**\ *grdfile*
    Name of the output grid to hold the interpolation.

.. _-I:

.. include:: explain_-I.rst_

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rgeo.rst_

Optional Arguments
------------------

.. _-D:

**-D**\ [*east*]
    Skip duplicate points since the spherical gridding algorithm cannot handle them.
    [Default assumes there are no duplicates, except possibly at the poles].
    Append a repeating longitude (*east*) to skip records with that longitude instead
    of the full (slow) search for duplicates.

.. _-Q:

**-Q**\ *mode*\ [*options*]
    Specify one of four ways to calculate tension factors to preserve
    local shape properties or satisfy arc constraints [Default is no
    tension].
**-Qp**
    Use **p**\ iecewise linear interpolation; no tension is applied.
**-Ql**
    Smooth interpolation with **l**\ ocal gradient estimates.
**-Qg**\ [*N*/*M*/*U*]
    Smooth interpolation with **g**\ lobal gradient estimates. You may
    optionally append *N*/*M*/*U*, where *N* is the number of
    iterations used to converge at solutions for gradients when variable
    tensions are selected (e.g., **-T** only) [3], *M* is the number of
    Gauss-Seidel iterations used when determining the global gradients
    [10], and *U* is the maximum change in a gradient at the last
    iteration [0.01].
**-Qs**\ [*E*/*U*/*N*]
    Use **s**\ moothing. Optionally append *E*/*U*/*N* [/0/0/3], where *E* is Expected
    squared error in a typical (scaled) data value, and *U* is Upper
    bound on weighted sum of squares of deviations from data. Here, *N* is the number of
    iterations used to converge at solutions for gradients when variable
    tensions are selected (e.g., **-T** only) [3]

.. _-T:

**-T**
    Use variable tension (ignored with **-Q**\ 0 [constant]

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**
    Before interpolation, scale data by the maximum data range [no
    scaling].

.. |Add_-bi| replace:: [Default is 3 input columns].
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-qi.rst_

.. include:: explain_colon.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

Examples
--------

.. include:: explain_example.rst_

To interpolate data from the remote file mars370d.txt using the piecewise
method for a 1x1 global grid, then plot it, try::

    gmt begin mars
      gmt sphinterpolate @mars370d.txt -Rg -I1 -Qp -Gmars.nc
      gmt grdimage mars.nc -JH0/4.5i -B30g30
    gmt end

To interpolate the points in the file testdata.txt on a global 1x1
degree grid with no tension, use::

    gmt sphinterpolate testdata.txt -Rg -I1 -Gsolution.nc

Notes
-----

The STRIPACK algorithm and implementation expect that there are no duplicate points
in the input.  It is best that the user ensures that this is the case.  GMT has tools,
such as :doc:`blockmean` and others, to combine close points into single entries.
Also, **sphinterpolate** has a **-D** option to determine and exclude duplicates, but
it is a very brute-force yet exact comparison that is very slow for large data sets.
A much quicker check involves appending a specific repeating longitude value.
Detection of duplicates in the STRIPACK library will exit the module.

See Also
--------

:doc:`gmt`,
:doc:`greenspline`,
:doc:`nearneighbor`,
:doc:`sphdistance`,
:doc:`sphtriangulate`,
:doc:`surface`,
:doc:`triangulate`

References
----------

Renka, R, J., 1997, Algorithm 772: STRIPACK: Delaunay Triangulation and
Voronoi Diagram on the Surface of a Sphere, *AMC Trans. Math. Software*,
**23**\ (3), 416-434.

Renka, R, J,, 1997, Algorithm 773: SSRFPACK: Interpolation of scattered
data on the Surface of a Sphere with a surface under tension, *AMC
Trans. Math. Software*, **23**\ (3), 435-442.
