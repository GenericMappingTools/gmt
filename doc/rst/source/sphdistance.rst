.. index:: ! sphdistance

***********
sphdistance
***********

.. only:: not man

    Create Voronoi distance, node, or natural nearest-neighbor grid on a sphere

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt sphdistance** [ *table* ] |-G|\ *grdfile*
[ |-C| ]
[ |-E|\ **d**\ \|\ **n**\ \|\ **z**\ [*dist*] ]
[ |SYN_OPT-I| ]
[ |-L|\ *unit* ]
[ |-N|\ *nodetable* ]
[ |-Q|\ *voronoi.txt* ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-j| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

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

.. _-G:

**-G**\ *grdfile*
    Name of the output grid to hold the computed distances (but see **-E**
    for other node value options).

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-C:

**-C**
    For large data sets you can save some memory (at the expense of more
    processing) by only storing one form of location coordinates
    (geographic or Cartesian 3-D vectors) at any given time, translating
    from one form to the other when necessary [Default keeps both arrays
    in memory]. Not applicable with **-Q**.

.. _-E:

**-Ed**\ \|\ **n**\ \|\ **z**\ [*dist*]
    Specify the quantity that should be assigned to the grid nodes.  By
    default we compute distances to the nearest data point [**-Ed**].
    Use **-En** to assign the ID numbers of the Voronoi polygons that each
    grid node is inside, or use **-Ez** for a natural nearest-neighbor grid where
    we assign all nodes inside the polygon the z-value of the center node.
    Optionally, append the resampling interval along Voronoi arcs in spherical
    degrees [1].

.. _-I:

.. include:: explain_-I.rst_

.. _-L:

**-L**\ *unit*
    Specify the unit used for distance calculations. Choose among **d**
    (spherical degree), **e** (m), **f** (feet), **k** (km), **M**
    (mile), **n** (nautical mile) or **u** survey foot.

.. _-N:

**-N**\ *nodetable*
    Read the information pertaining to each Voronoi
    polygon (the unique node lon, lat and polygon area) from a separate
    file [Default acquires this information from the ASCII segment
    headers of the output file]. Required if binary input via **-Q** is used.

.. _-Q:

**-Q**\ *voronoi.txt*
    Append the name of a file with pre-calculated Voronoi polygons
    [Default performs the Voronoi construction on input data]. For
    binary data **-bi** you must specify the node
    information separately (via **-N**).

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rgeo.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_distcalc.rst_

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

To generate the same grid in two steps using :doc:`sphtriangulate` separately, try

   ::

    gmt sphtriangulate testdata.txt -Qv > voronoi.txt
    gmt sphdistance -Qvoronoi.txt -Rg -I1 -Gglobedist.nc

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
