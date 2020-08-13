.. index:: ! gmtcount
.. include:: module_core_purpose.rst_

********
gmtcount
********

|gmtcount_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt gmtcount** [ *table* ] |-G|\ *out_grdfile*
|SYN_OPT-I|
|-C|\ **a**\|\ **b**\|\ **d**\|\ **l**\|\ **L**\|\ **m**\|\ **n**\|\ **o**\|\ **p**\|\ **q**\ [*quant*]\|\ **u**\|\ **U**\|\ **r**\|\ **s**
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

**gmtcount** reads arbitrarily located (x,y[,z][,w]) points
(2-4 columns) from standard input [or *table*] and determines
which nodes in the specified grid layout are within the given
radius.  The point is then considered in the calculation of the
specified statistic per node.

Required Arguments
------------------

.. _-C:

**-C**\ **a**\|\ **b**\|\ **d**\|\ **i**\|\ **l**\|\ **L**\|\ **m**\|\ **n**\|\ **o**\|\ **p**\|\ **q**\ [*quant*]\|\ **u**\|\ **U**\|\ **r**\|\ **s**
    Choose the statistic that will be computed per node based on the points that
    are within *radius* distance of the node.  Select one of **a** for mean (average),
    **b** for median absolute deviation (MAD), **d** for standard deviation, **i** for 25-75% interquartile range,
    **l** for minimum (low), **L** for minimum of positive values,
    **m** for median, **n** the number of values, **o** for LMS scale,
    **p** for mode (maximum likelihood), **q** for selected quantile
    (append desired quantile in 0-100% range [50]), **r** for full (max-min) range,
    **u** for maximum (upper), **U** for maximum of negative values,
    or **s** for the sum.

.. _-G:

**-G**\ *out_grdfile*
    Give the name of the output grid file.

.. _-I:

.. include:: explain_-I.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. _-S:

**-S**\ *search_radius*
    Sets the *search_radius* that determines which data points are
    considered close to a node. Append the distance unit (see `Units`_).

Optional Arguments
------------------

*table*
    A 2-4 column ASCII file(s) [or binary, see
    **-bi**] holding (x,y[,z][,w]) data values. You must use **-W**
    to indicate you have weights.  Only **-Cn** will accept 2 columns only.
    If no file is specified, **gmtcount** will read from standard input.

.. _-E:

**-E**\ *empty*
    Set the value assigned to empty nodes [NaN].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**
   Input data have an extra column containing observation point weights.
   If weights are given then weighted statistical quantities will be computed
   while the count will be the sum of the weights instead of number of points.


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
      gmt gmtcount @ship_15.txt -R245/255/20/30 -I5m -Ggrid.nc -S15m
      gmt grdimage grid.nc -B
    gmt end show

To create a gridded data set from the file seaMARCII_bathy.lon_lat_z
using a 0.5 min grid, a 5 km search radius, using an octant search with
100% sector coverage, and set empty nodes to -9999::

    gmt gmtcount seaMARCII_bathy.lon_lat_z -R242/244/-22/-20 -I0.5m -E-9999 -Gbathymetry.nc -S5k -N8+m8

To make a global grid file from the data in geoid.xyz using a 1 degree
grid, a 200 km search radius, spherical distances, using an quadrant
search, and set nodes to NaN only when fewer than two quadrants contain
at least one value::

    gmt gmtcount geoid.xyz -R0/360/-90/90 -I1 -Lg -Ggeoid.nc -S200k -N4

See Also
--------

:doc:`blockmean`,
:doc:`blockmedian`,
:doc:`blockmode`, :doc:`gmt`,
:doc:`greenspline`,
:doc:`nearneighbor`,
:doc:`sphtriangulate`,
:doc:`surface`,
:doc:`triangulate`
