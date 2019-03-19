.. index:: ! fitcircle

*********
fitcircle
*********

.. only:: not man

    find mean position and great [or small] circle fit to points on a sphere.

Synopsis
--------

.. include:: common_SYN_OPTs.rst_


**gmt fitcircle** [ *table* ] |-L|\ *norm* [ |-F|\ *flags* ] [ |-S|\ [*lat*] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**fitcircle** reads lon,lat [or lat,lon] values from the first two
columns on standard input [or *table*]. These are converted to
Cartesian three-vectors on the unit sphere. Then two locations are
found: the mean of the input positions, and the pole to the great circle
which best fits the input positions. The user may choose one or both of
two possible solutions to this problem. The first is called **-L1** and
the second is called **-L2**. When the data are closely grouped along a
great circle both solutions are similar. If the data have large
dispersion, the pole to the great circle will be less well determined
than the mean. Compare both solutions as a qualitative check.

The **-L1** solution is so called because it approximates the
minimization of the sum of absolute values of cosines of angular
distances. This solution finds the mean position as the Fisher average
of the data, and the pole position as the Fisher average of the
cross-products between the mean and the data. Averaging cross-products
gives weight to points in proportion to their distance from the mean,
analogous to the "leverage" of distant points in linear regression in the plane.

The **-L2** solution is so called because it approximates the
minimization of the sum of squares of cosines of angular distances. It
creates a 3 by 3 matrix of sums of squares of components of the data
vectors. The eigenvectors of this matrix give the mean and pole
locations. This method may be more subject to roundoff errors when there
are thousands of data. The pole is given by the eigenvector
corresponding to the smallest eigenvalue; it is the least-well
represented factor in the data and is not easily estimated by either method.

Required Arguments
------------------

.. _-L:

**-L**\ *norm*
    Specify the desired *norm* as 1 or 2, or use **-L** or **-L3** to
    see both solutions.

Optional Arguments
------------------

*table*
    One or more ASCII [or binary, see **-bi**] files containing lon,lat [or lat,lon; see
    **-:**\ [**i**\ \|\ **o**]] values in the first 2 columns. If no
    file is specified, **fitcircle** will read from standard input.

.. _-F:

**-F**\ *flags*
    Traditionally, **fitcircle** will write its results in the form of a text report, with
    the values intermingled with report sentences.  Use **-F** to only return data
    coordinates, and append *flags* to specify which coordinates you would like. You
    can choose one or more items from **f** (Flat Earth mean location), **m** (mean location),
    **n** (north pole of great circle), **s** (south pole of great circle), and
    **c** (pole of small circle and its colatitude, which requires **-S**).

.. _-S:

**-S**\ [*lat*]
    Attempt to fit a small circle instead of a great circle. The pole
    will be constrained to lie on the great circle connecting the pole
    of the best-fit great circle and the mean location of the data.
    Optionally append the desired fixed latitude of the small circle
    [Default will determine the optimal latitude].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

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

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_


Examples
--------

Suppose you have lon,lat,grav data along a twisty ship track in the file
ship.xyg. You want to project this data onto a great circle and resample
it in distance, in order to filter it or check its spectrum. Do the
following:

   ::

    gmt fitcircle ship.xyg -L2
    gmt project ship.xyg -Cox/oy -Tpx/py -S -Fpz | sample1d -S-100 -I1 > output.pg

Here, *ox*/*oy* is the lon/lat of the mean from **fitcircle**, and
*px*/*py* is the lon/lat of the pole. The file output.pg has distance,
gravity data sampled every 1 km along the great circle which best fits
ship.xyg

If you have lon, lat points in the file data.txt and wish to return the northern
hemisphere great circle pole location using the L2 norm, try

   ::

    gmt fitcircle data.txt -L2 -Fn > pole.txt


See Also
--------

:doc:`gmt`,
:doc:`gmtvector`,
:doc:`project`,
:doc:`mapproject`,
:doc:`sample1d`
