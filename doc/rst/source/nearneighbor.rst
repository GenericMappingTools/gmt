.. index:: ! nearneighbor
.. include:: module_core_purpose.rst_

************
nearneighbor
************

|nearneighbor_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt nearneighbor** [ *table* ] |-G|\ *outgrid*
|SYN_OPT-I|
|-N|\ *sectors*\ [**+m**\ *min_sectors*]\ \|\ **n**
|SYN_OPT-R|
|-S|\ *search_radius*
[ |-E|\ *empty* ]
[ |SYN_OPT-V| ]
[ |-W| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**nearneighbor** reads arbitrarily located (*x,y,z*\ [,\ *w*]) triples
[quadruplets] from standard input [or *table*] and uses a nearest
neighbor algorithm to assign a weighted average value to each node that
has one or more data points within a search radius (*R*) centered on the
node with adequate coverage across a subset of the chosen sectors. The
node value is computed as a weighted mean of the nearest point from each
sector inside the search radius. The weighting function and the averaging
used is given by


.. math::

    w(r_i) = \frac{w_i}{1 + d(r_i) ^ 2}, \quad d(r) = \frac {3r}{R}, \quad \bar{z} = \frac{\sum_i^n w(r_i) z_i}{\sum_i^n w(r_i)}

where *n* is the number of data points that satisfy the selection criteria and
:math:`r_i` is the distance from the node to the *i*'th data point. If no data
weights are supplied then :math:`w_i = 1`.

.. figure:: /_images/GMT_nearneighbor.*
   :width: 300 px
   :align: center

   Search geometry includes the search radius (R) which limits the points
   considered and the number of sectors (here 4), which restricts how points inside
   the search radius contribute to the value at the node.  Only the closest point
   in each sector (red circles) contribute to the weighted estimate.

Required Arguments
------------------

*table*
    3 [or 4, see **-W**] column ASCII file(s) [or binary, see
    **-bi**] holding (*x,y,z*\ [,\ *w*]) data values. If
    no file is specified, **nearneighbor** will read from standard input.

.. _-G:

**-G**\ *outgrid*
    Give the name of the output grid file.

.. _-I:

.. include:: explain_-I.rst_

.. _-N:

**-N**\ *sectors*\ [**+m**\ *min_sectors*]\|\ **n**
    The circular search area centered on each node is divided into *sectors*
    sectors. Average values will only be computed if there is *at least*
    one value inside each of at least *min_sectors* of the sectors for a given
    node. Nodes that fail this test are assigned the value NaN (but see
    **-E**). If **+m** is omitted then *min_sectors* is set to be at least 50%
    of *sectors* (i.e., rounded up to next integer) [Default is a quadrant
    search with 100% coverage, i.e., *sectors* = *min_sectors* = 4]. Note
    that only the nearest value per sector enters into the averaging; the
    more distant points are ignored.  Alternatively, use **-Nn** to call
    GDALʻs nearest neighbor algorithm instead.

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. _-S:

**-S**\ *search_radius*
    Sets the *search_radius* that determines which data points are
    considered close to a node. Append the distance unit (see `Units`_).

Optional Arguments
------------------

.. _-E:

**-E**\ *empty*
    Set the value assigned to empty nodes [NaN].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**
   Input data have a 4th column containing observation point weights.
   These are multiplied with the geometrical weight factor to determine
   the actual weights used in the calculations.

.. |Add_-bi| replace:: [Default is 3 (or 4 if **-W** is set) columns].
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| replace:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

**-n**\ [**b**\|\ **c**\|\ **l**\|\ **n**][**+a**][**+b**\ *BC*][**+t**\ *threshold*]
   Append **+b**\ *BC* to set any boundary conditions to be used,
   adding **g** for geographic, **p** for periodic, or **n** for
   natural boundary conditions. For the latter two you may append **x**
   or **y** to specify just one direction, otherwise both are assumed.
   [Default is geographic if grid is geographic].

.. include:: explain_-qi.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_float.rst_

Examples
--------

.. include:: explain_example.rst_

To grid the data in the remote file @ship_15.txt at 5x5 arc minutes using
a search radius of 15 arch minutes, and plot the resulting grid using
default projection and colors, try::

    gmt begin map
      gmt nearneighbor @ship_15.txt -R245/255/20/30 -I5m -Ggrid.nc -S15m
      gmt grdimage grid.nc -B
    gmt end show

To create a gridded data set from the file seaMARCII_bathy.lon_lat_z
using a 0.5 min grid, a 5 km search radius, using an octant search with
100% sector coverage, and set empty nodes to -9999::

    gmt nearneighbor seaMARCII_bathy.lon_lat_z -R242/244/-22/-20 -I0.5m -E-9999 -Gbathymetry.nc -S5k -N8+m8

To make a global grid file from the data in geoid.xyz using a 1 degree
grid, a 200 km search radius, spherical distances, using an quadrant
search, and set nodes to NaN only when fewer than two quadrants contain
at least one value::

    gmt nearneighbor geoid.xyz -R0/360/-90/90 -I1 -Lg -Ggeoid.nc -S200k -N4

See Also
--------

:doc:`blockmean`,
:doc:`blockmedian`,
:doc:`blockmode`, :doc:`gmt`,
:doc:`greenspline`,
:doc:`sphtriangulate`,
:doc:`surface`,
:doc:`triangulate`
