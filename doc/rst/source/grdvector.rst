.. index:: ! grdvector

*********
grdvector
*********

.. only:: not man

    grdvector - Plot vector field from two component grids

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**grdvector** *compx.nc* *compy.nc* **-J**\ *parameters* [ **-A** ]
[ |SYN_OPT-B| ]
[ **-G**\ *fill* ]
[ |SYN_OPT-I| ]
[ **-K** ] [ **-N** ] [ **-O** ] [ **-P** ] [ **-Q**\ *parameters* ]
[ |SYN_OPT-R| ]
[ **-S**\ [**i**\ \|\ **l**\ ]\ *scale* ] [ **-T** ]
[ |SYN_OPT-U| ]
[ **-W**\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ **-Z** ]
[ |SYN_OPT-c| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]

|No-spaces|

Description
-----------

**grdvector** reads two 2-D grid files which represents the x- and
y-components of a vector field and produces a vector field plot by
drawing vectors with orientation and length according to the information
in the files. Alternatively, polar coordinate components may be used (r,
theta). **grdvector** is basically a short-hand for using 2 calls to
:doc:`grd2xyz` and pasting the output through **psxy -SV**. 

Required Arguments
------------------

*compx.nc*
    Contains the x-component of the vector field.
*compy.nc*
    Contains the y-component of the vector field. (See GRID FILE FORMATS below.) 

.. include:: explain_-J.rst_

Optional Arguments
------------------

**-A**
    Means grid files have polar (r, theta) components instead of
    Cartesian (x, y). 

.. include:: explain_-B.rst_

**-C**\ [*cptfile*]
    Use *cptfile* to assign colors based on vector length.  Alternatively,
    supply the name of a GMT color master CPT [rainbow] and let
    **grdvector** automatically determine a 16-level continuous CPT from
    the grid's z-range.
**-G**\ *fill*
    Sets color or shade for vector interiors [Default is no fill].
**-I**
    Only plot vectors at nodes every *x\_inc*, *y\_inc* apart (must be
    multiples of original grid spacing). Append **m** for arc minutes or
    **s** for arc seconds. [Default plots every node]. 

.. include:: explain_-K.rst_

**-N**
    Do NOT clip vectors at map boundaries [Default will clip]. 

.. include:: explain_-O.rst_

.. include:: explain_-P.rst_

**-Q**\ *parameters*
    Modify vector parameters. For vector heads, append vector head
    *size* [Default is 0, i.e., stick-plot]. See VECTOR ATTRIBUTES for
    specifying additional attributes. 

.. |Add_-R| replace:: Specify a subset of the grid.
.. include:: explain_-R.rst_

**-S**\ [**i**\ \|\ **l**\ ]\ *scale*
    Sets scale for Cartesian vector length in data units per distance measurement
    unit [1]. Append **c**, **i**, or **p** to indicate the measurement
    unit (cm, inch,or point). Prepend **l** to indicate a fixed length
    for all vectors.  For Geographic data, give scale in data units per
    km. Use **-Si** if it is simpler to give the reciprocal scale in
    measurement unit per data unit or km per data unit.

**-T**
    Means azimuth of Cartesian data sets should be adjusted for different scales
    in the x- and y-directions [Leave alone].

.. include:: explain_-U.rst_

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

**-W**\ *pen*
    Set pen attributes used for vector outlines [Default: width =
    default, color = black, style = solid]. 

.. include:: explain_-XY.rst_

**-Z**
    Means the angles provided are azimuths rather than direction
    (requires **-A**). 

.. include:: explain_-c.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout.rst_

.. include:: explain_vectors.rst_


Examples
--------

To draw the vector field given by the files r.nc and theta.nc on a
linear plot with scale 5 cm per data unit, using vector rather than
stick plot, scale vector magnitudes so that 10 units equal 1 inch, and
center vectors on the node locations, run

   ::

    gmt grdvector r.nc theta.nc -Jx5c -A -Q0.1i+e+jc -S10i > gradient.ps

To plot a geographic data sets given the files com_x.nc and comp_y.nc,
using a scale of 200 km per data unit, try

   ::

    gmt grdvector comp_x.nc comp_y.nc -JH0/20c -Q0.1i+e+jc -S200 > globe.ps

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`grdcontour`, :doc:`psxy`
