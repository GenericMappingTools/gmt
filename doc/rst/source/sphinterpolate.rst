.. index:: ! sphinterpolate

**************
sphinterpolate
**************

.. only:: not man

    sphinterpolate - Spherical gridding in tension of data on a sphere

`Synopsis <#toc1>`_
-------------------

.. include:: common_SYN_OPTs.rst_

**sphinterpolate** [ *table* ] **-G**\ *grdfile*
[ |SYN_OPT-I| ]
[ **-Q**\ *mode*\ [/*options*] ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ **-Z** ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ **-r** ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**sphinterpolate** reads one or more ASCII [or binary] files (or
standard input) containing lon, lat, f and performs a Delaunay
triangulation to set up a spherical interpolation in tension. The final
grid is saved to the specified file. Several options may be used to
affect the outcome, such as choosing local versus global gradient
estimation or optimize the tension selection to satisfy one of four
criteria.

Required Arguments
------------------

**-G**\ *grdfile*
    Name of the output grid to hold the interpolation.

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. include:: explain_-I.rst_

**-Q**\ *mode*\ [/*options*]
    Specify one of four ways to calculate tension factors to preserve
    local shape properties or satisfy arc constraints [Default is no
    tension].
**-Q**\ 0
    Piecewise linear interpolation; no tension is applied.
**-Q**\ 1
    Smooth interpolation with local gradient estimates.
**-Q**\ 2
    Smooth interpolation with global gradient estimates. You may
    optionally append /*N*/*M*/*U*, where *N* is the number of
    iterations used to converge at solutions for gradients when variable
    tensions are selected (e.g., **-T** only) [3], *M* is the number of
    Gauss-Seidel iterations used when determining the global gradients
    [10], and *U* is the maximum change in a gradient at the last
    iteration [0.01].
**-Q**\ 3
    Smoothing. Optionally append */E/U* [/0/0], where *E* is Expected
    squared error in a typical (scaled) data value, and *U* is Upper
    bound on weighted sum of squares of deviations from data.

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rgeo.rst_

**-T**
    Use variable tension (ignored with **-Q**\ 0 [constant]

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

**-Z**
    Before interpolation, scale data by the maximum data range [no
    scaling].

.. |Add_-bi| replace:: [Default is 3 input columns].
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: explain_-bo.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_colon.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_help.rst_

.. include:: explain_ascii_precision.rst_


Grid Values Precision
---------------------

Regardless of the precision of the input data, GMT programs that create
grid files will internally hold the grids in 4-byte floating point
arrays. This is done to conserve memory and furthermore most if not all
real data can be stored using 4-byte floating point values. Data with
higher precision (i.e., double precision values) will lose that
precision once GMT operates on the grid or writes out new grids. To
limit loss of precision when processing data you should always consider
normalizing the data prior to processing.

Examples
--------

To interpolate the points in the file testdata.txt on a global 1x1
degree grid with no tension, use

    sphinterpolate testdata.txt -Rg -I1 -Gsolution.nc

See Also
--------

`GMT <GMT.html>`_, `greenspline <greenspline.html>`_
`sphdistance <sphdistance.html>`_
`sphtriangulate <sphtriangulate.html>`_
`triangulate <triangulate.html>`_

`References <#toc10>`_
----------------------

Renka, R, J., 1997, Algorithm 772: STRIPACK: Delaunay Triangulation and
Voronoi Diagram on the Surface of a Sphere, *AMC Trans. Math. Software*,
**23**\ (3), 416-434.

Renka, R, J,, 1997, Algorithm 773: SSRFPACK: Interpolation of scattered
data on the Surface of a Sphere with a surface under tension, *AMC
Trans. Math. Software*, **23**\ (3), 435-442.
