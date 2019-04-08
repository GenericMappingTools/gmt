.. index:: ! grdmask

*******
grdmask
*******

.. only:: not man

    grdmask - Create mask grid from polygons or point coverage

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdmask** *pathfiles* |-G|\ *mask_grd_file*
|SYN_OPT-I|
|SYN_OPT-R|
[ |-A|\ [**m**\ \|\ **p**\ \|\ **x**\ \|\ **y**] ]
[ |-N|\ [**z**\ \|\ **Z**\ \|\ **p**\ \|\ **P**]\ *values* ]
[ |-S|\ *search\_radius*\ [*unit*] \|\ *xlim*\ /*ylim* ] [ |SYN_OPT-V| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-j| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdmask** can operate in two different modes. 1. It reads one or more
*pathfiles* that each define a closed polygon. The nodes defined by the
specified region and lattice spacing will be set equal to one of three
possible values depending on whether the node is outside, on the polygon
perimeter, or inside the polygon. The resulting mask may be used in
subsequent operations involving :doc:`grdmath` to mask out data from
polygonal areas. 2. The *pathfiles* simply represent data point locations
and the mask is set to the inside or outside value depending on whether
a node is within a maximum distance from the nearest data point. If the
distance specified is zero then only the nodes nearest each data point
are considered "inside". 

Required Arguments
------------------

*pathfiles*
    The name of 1 or more ASCII [or binary, see
    **-bi**] files holding the polygon(s) or data points.

.. _-G:

**-G**\ *mask_grd_file*]
    Name of resulting output mask grid file. (See GRID FILE FORMATS below). 

.. _-I:

.. include:: explain_-I.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ [**m**\ \|\ **p**\ \|\ **x**\ \|\ **y**]
    If the input data are geographic (as indicated by **-f**) then the
    sides in the polygons will be approximated by great circle arcs.
    When using the **-A** sides will be regarded as straight lines.
    Alternatively, append **m** to have sides first follow meridians,
    then parallels. Or append **p** to first follow parallels, then meridians.
    For Cartesian data, points are simply connected, unless you append
    **x** or **y** to construct stair-case paths whose first move is along 
    *x* or *y*, respectively.

.. _-N:

**-N**\ [**z**\ \|\ **Z**\ \|\ **p**\ \|\ **P**]\ *values*
    Sets the *out/edge/in* that will be assigned to nodes that are
    *out*\ side the polygons, on the *edge*, or *in*\ side. Values can
    be any number, including the textstring NaN [Default is 0/0/1].
    Optionally, use **Nz** to set polygon insides to the z-value
    obtained from the data (either segment header **-Z**\ *zval*,
    **-L**\ *header* or via **-a**\ Z=\ *name*); use **-NZ** to consider
    the polygon boundary as part of the inside. Alternatively, use
    **-Np** to use a running number as polygon ID; optionally append
    start of the sequence [0]. Here, **-NP** includes the polygon
    perimeter as inside. Note:
    **-N**\ **z**\ \|\ **Z**\ \|\ **p**\ \|\ **P** cannot be used in
    conjunction with **-S**; they also all optionally accept /*out* [0].

.. _-S:

**-S**\ *search\_radius*\ [*unit*] \|\ *xlim*\ /*ylim* 
    Set nodes to inside, on edge, or outside depending on their distance
    to the nearest data point. Nodes within *radius* [0] from the
    nearest data point are considered inside; append a distance unit
    (see :ref:`Unit_attributes`). If *radius* is given as **z** then we instead read
    individual radii from the 3rd input column.  Unless Cartesian data,
    specify the unit of these radii by appending it after **-Sz**.
    If **-S** is not set then we consider the input data to define
    one or more closed polygon(s) instead.  For Cartesian data with
    different units you can instead append *xlim*\ /*ylim* which will
    perform a rectangular search where all nodes within ±\ *xlim* and
    ±\ *ylim* of a data point will be considered inside.
    One can also achieve the rectangular selection effect by using the **-S**\ *n_cells*\ **c** form.
    Here *n_cells* means the number of cells around each data point. As an example, **-S**\ 0\ **c** means
    that only the cell where point lies is masked, **-S**\ 1\ **c** masks one cell beyond that
    (i.e. makes a 3x3 neighborhood), and so on.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 input columns (3 with **-Sz**)]. 
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

.. include:: explain_distcalc.rst_

**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
   Append **+b**\ *BC* to set any boundary conditions to be used,
   adding **g** for geographic, **p** for periodic, or **n** for
   natural boundary conditions. For the latter two you may append **x**
   or **y** to specify just one direction, otherwise both are assumed.
   [Default is geographic if grid is geographic]. 

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_core.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_grd_output.rst_

.. include:: explain_grd_coord.rst_

.. include:: explain_inside.rst_

Notes
-----

A grid produced by grdmask is a *categorical* dataset.  As such,
one has to be careful not to interpolate it with standard methods,
such as splines.  However, if you make a map of this grid using
a map projection the grid will be reprojected to yield a rectangular
matrix in the projected coordinates.  This interpolation is done
using splines by default and thus may yield artifacts in your map.
We recommend you use :doc:`grdimage` **-nn** to instead use a nearest
neighbor interpolation for such cases.

Save storage space
------------------

Since most uses of grdmask revolves around creating mask grids that hold just a few integer
values (and perhaps NaN), we choose to write them to disk as byte grids by appending the
suffix **=nb** to the desired grid filename.  Some situations may store integers that exceed
the range available in a byte and for those we specify a short integer grid with **=ns**.
For larger integers you may consider **=ni**, otherwise use the default float grid format.

Examples
--------

To set all nodes inside and on the polygons coastline_*.xy to 0, and
outside points to 1, do

   ::

    gmt grdmask coastline_*.xy -R-60/-40/-40/-30 -I5m -N1/0/0 -Gland_mask.nc=nb -V

To set nodes within 50 km of data points to 1 and other nodes to NaN, do

   ::

    gmt grdmask data.xyz -R-60/-40/-40/-30 -I5m -NNaN/1/1 -S50k -Gdata_mask.nc=nb -V

To assign polygon IDs to the gridnodes using the insides of the polygons
in plates.gmt, based on the attribute POL_ID, do

   ::

    gmt grdmask plates.gmt -R-40/40/-40/40 -I2m -Nz -Gplate_IDs.nc=ns -aZ=POL_ID -V

Same exercise, but instead compute running polygon IDs starting at 100, do

   ::

    gmt grdmask plates.gmt -R-40/40/-40/40 -I2m -Np100 -Gplate_IDs.nc=ns -V

See Also
--------

:doc:`gmt`, :doc:`grdlandmask`,
:doc:`grdmath`, :doc:`grdclip`,
:doc:`mask`, :doc:`clip`
