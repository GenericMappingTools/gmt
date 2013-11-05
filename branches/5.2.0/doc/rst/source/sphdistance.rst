.. index:: ! sphdistance

***********
sphdistance
***********

.. only:: not man

    sphdistance - Make grid of distances to nearest points on a sphere

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**sphdistance** [ *table* ] **-G**\ *grdfile* [ **-C** ] [ **-E** ]
[ |SYN_OPT-I| ]
[ **-L**\ *unit* ] [ **-Q**\ *voronoi.d* ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ **-r** ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**sphdistance** reads one or more ASCII [or binary] files (or standard
input) containing lon, lat and performs the construction of Voronoi
polygons. These polygons are then processed to calculate the nearest
distance to each node of the lattice and written to the specified grid.
The Voronoi algorithm used is STRIPACK. As an option, you may provide
pre-calculated Voronoi polygon file in the format written by
:doc:`sphtriangulate`, thus bypassing the memory- and time-consuming
triangularization.

Required Arguments
------------------

**-G**\ *grdfile*
    Name of the output grid to hold the computed distances.

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

**-C**
    For large data set you can save some memory (at the expense of more
    processing) by only storing one form of location coordinates
    (geographic or Cartesian 3-D vectors) at any given time, translating
    from one form to the other when necessary [Default keeps both arrays
    in memory]. Not applicable with **-Q**.
**-E**
    Instead of computing distances, return the ID numbers of the Voronoi
    polygons that each grid node is inside [Default computes distances].

.. include:: explain_-I.rst_

**-L**\ *unit*
    Specify the unit used for distance calculations. Choose among **d**
    (spherical degree), **e** (m), **f** (feet), **k** (km), **M**
    (mile), **n** (nautical mile) or **u** survey foot. A spherical
    approximation is used unless :ref:`PROJ_ELLIPSOID <Projection Parameters>` is set to an actual
    ellipsoid. **-N** Read the information pertaining to each Voronoi
    polygon (the unique node lon, lat and polygon area) from a separate
    file [Default acquires this information from the ASCII segment
    headers of the output file]. Required if binary input via **-Q** is used.

**-Q**\ *voronoi.d*
    Append the name of a file with pre-calculated Voronoi polygons
    [Default performs the Voronoi construction on input data]. For
    binary data **-bi** you must specify the node
    information separately (via **-N**).

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rgeo.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: explain_-bo.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_colon.rst_
.. include:: explain_help.rst_
.. include:: explain_precision.rst_
.. include:: explain_float.rst_

Examples
--------

To construct Voronoi polygons from the points in the file testdata.txt
and then calculate distances from the data to a global 1x1 degree grid, use

   ::

    gmt sphdistance testdata.txt -Rg -I1 -Gglobedist.nc

To generate the same grid in two steps using **sphtriangulate** separately, try

   ::

    gmt sphtriangulate testdata.txt -Qv > voronoi.d
    gmt sphdistance -Qvoronoi.d -Rg -I1 -Gglobedist.nc

See Also
--------

:doc:`gmt`,
:doc:`sphtriangulate`,
:doc:`triangulate`

References
----------

Renka, R, J., 1997, Algorithm 772: STRIPACK: Delaunay Triangulation and
Voronoi Diagram on the Surface of a Sphere, *AMC Trans. Math. Software*,
**23**\ (3), 416-434.
