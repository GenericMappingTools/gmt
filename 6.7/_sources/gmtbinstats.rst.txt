.. index:: ! gmtbinstats
.. include:: module_core_purpose.rst_

********
binstats
********

|gmtbinstats_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt binstats** [ *table* ] |-G|\ *outgrid*
|SYN_OPT-I|
|-C|\ **a**\|\ **d**\|\ **g**\|\ **i**\|\ **l**\|\ **L**\|\ **m**\|\ **n**\|\ **o**\|\ **p**\|\ **q**\ [*quant*]\|\ **r**\|\ **s**\|\ **u**\|\ **U**\|\ **z**
|SYN_OPT-R|
|-S|\ *radius*
[ |-E|\ *empty* ]
[ |-N| ]
[ |-T|\ [**h**\|\ **r**] ]
[ |SYN_OPT-V| ]
[ |-W|\ [**+s**] ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**binstats** reads arbitrarily located (*x, y*\ [, *z*][, *w*]) points
(2-4 columns) from standard input [or *table*] and for each
node in the specified grid layout determines which points are
within the given radius.  These points are then used in the
calculation of the specified statistic. The results may be
presented as is or may be normalized by the circle area to
perhaps give density estimates.  Alternatively, select
hexagonal tiling instead or a rectangular grid layout.

Required Arguments
------------------

*table*
    A 2-4 column ASCII file(s) [or binary, see
    **-bi**] holding (*x, y*\ [, *z*][, *w*]) data values. You must use |-W|
    to indicate that you have weights.  Only |-C|\ **n** will accept 2 columns only.
    If no file is specified, **binstats** will read from standard input.

.. _-C:

**-C**\ **a**\|\ **d**\|\ **g**\|\ **i**\|\ **l**\|\ **L**\|\ **m**\|\ **n**\|\ **o**\|\ **p**\|\ **q**\ [*quant*]\|\ **r**\|\ **s**\|\ **u**\|\ **U**\|\ **z**
    Choose the statistic that will be computed per node based on the points that
    are within *radius* distance of the node.  Append one directive among these candidates:

    - **a**: Mean (i.e., average).
    - **d**: Median absolute deviation (MAD).
    - **g**: The full (max-min) range.
    - **i**: The 25-75% interquartile range.
    - **l**: Minimum (lowest value).
    - **L**: Minimum of positive values only.
    - **m**: Median value.
    - **n**: The number of values per bin.
    - **o**: Least median square (LMS) scale.
    - **p**: Mode (maximum likelihood estimate).
    - **q**: Selected quantile (append desired *quantile* in 0-100% range [50]).
    - **r**: Root mean square (RMS).
    - **s**: Standard deviation.
    - **u**: Maximum (highest value).
    - **U**: Maximum of negative values only.
    - **z**: The sum of the values.

.. _-G:

.. |Add_outgrid| replace:: Give the name of the output grid file.
.. include:: /explain_grd_inout.rst_
    :start-after: outgrid-syntax-begins
    :end-before: outgrid-syntax-ends

.. _-I:

.. include:: explain_-I.rst_

.. |Add_-R| replace:: |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

Optional Arguments
------------------

.. _-E:

**-E**\ *empty*
    Set the value assigned to empty nodes [NaN].

.. _-N:

**-N**
    Normalize the resulting grid values by the area represented by the search *radius* [no normalization].

.. _-S:

**-S**\ *radius*
    Sets the search *radius* that determines which data points are
    considered close to a node. Append the distance unit (see `Units`_).
    Not compatible with |-T|.

.. _-T:

**-T**\ [**h**\|\ **r**]
    Instead of circular, possibly overlapping areas, select non-overlapping tiling.  Choose between
    **r**\ ectangular and **h**\ exagonal binning. For **-Tr**, set bin sizes via |-I| and we write
    the computed statistics to the grid file named in |-G|.  For **-Th**, we write a table with
    the centers of the hexagons and the computed statistics to standard output (or to the file named
    in |-G|).  Here, the |-I| setting is expected to set the *y* increment only and we compute
    the *x*-increment given the geometry. Because the horizontal spacing between hexagon centers in
    *x* and *y* have a ratio of :math:`\sqrt{3}`, we will automatically adjust *xmax* in |-R| to
    fit a whole number of hexagons. **Note**: Hexagonal tiling requires Cartesian data.

.. figure:: /_images/GMT_hexbinning.*
   :width: 500 px
   :align: center

   a) Hexagonal polygons (light blue and light red) used for binning. The red are all inside the
   gridding region while the blue are outside along the border. Yellow squares are test data,
   black nodes are grid nodes supported by a hexagon center, and while squares are nodes that fall 
   between. b) Grid showing result of hexagonal binning which yields a constant value inside each
   polygon.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [**+s**]
   Input data have an extra column containing observation point weight.
   If weights are given then weighted statistical quantities will be computed
   while the count will be the sum of the weights instead of number of points.
   If your weights are actually uncertainties (:math:`1\sigma`) then append **+s**
   and we compute weight = :math:`\frac{1}{\sigma}`.

.. include:: explain_-aspatial.rst_

.. |Add_-bi| replace:: [Default is 3 (or 4 if |-W| is set) columns].
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

.. include:: explain_-qi.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_-w.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_float.rst_

Examples
--------

.. include:: explain_example.rst_

To examine the population inside a circle of 1000 km radius for all nodes in a 5x5 arc degree grid,
using the remote file @capitals.gmt, and plot the resulting grid using default projection and colors, try::

    gmt begin map
      gmt binstats @capitals.gmt -a2=population -Rg -I5 -Cz -Gpop.nc -S1000k
      gmt grdimage pop.nc -B
    gmt end show

To do hexagonal binning of the data in the file mydata.txt and counting the number of points inside
each hexagon, try::

    gmt binstats mydata.txt -R0/5/0/3 -I1 -Th -Cn > counts.txt

See Also
--------

:doc:`blockmean`,
:doc:`blockmedian`,
:doc:`blockmode`, :doc:`gmt`,
:doc:`nearneighbor`,
:doc:`triangulate`,
:doc:`xyz2grd`
