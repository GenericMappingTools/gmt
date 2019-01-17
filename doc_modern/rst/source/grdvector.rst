.. index:: ! grdvector

*********
grdvector
*********

.. only:: not man

    Plot vector field from two component grids

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdvector** *compx.nc* *compy.nc* **-J**\ *parameters* [ |-A| ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-G|\ *fill* ]
[ |-I|\ [**x**]\ *dx*\ [/*dy*] ]
[ |-N| ] [ |-Q|\ *parameters* ]
[ |SYN_OPT-R| ]
[ |-S|\ [**i**\ \|\ **l**\ ]\ *scale*\ [*unit*] ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdvector** reads two 2-D grid files which represents the *x*\ - and
*y*\ -components of a vector field and produces a vector field plot by
drawing vectors with orientation and length according to the information
in the files. Alternatively, polar coordinate *r*, *theta* grids may be given
instead.

Required Arguments
------------------

*compx.nc*
    Contains the x-components of the vector field.
*compy.nc*
    Contains the y-components of the vector field. (See GRID FILE FORMATS below.) 

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

Optional Arguments
------------------

.. _-A:

**-A**
    The grid files contain polar (r, theta) components instead of
    Cartesian (x, y) [Default is Cartesian components]. 

.. _-B:

.. include:: explain_-B.rst_

.. _-C:

**-C**\ [*cpt* \|\ *master*\ [**+i**\ *zinc*] \|\ *color1,color2*\ [,\ *color3*,...]]
    Use *cpt* to assign colors based on vector length. Alternatively,
    supply the name of a GMT color master dynamic CPT [rainbow] to
    automatically determine a continuous CPT from
    the grid's z-range; you may round up/down the z-range by adding **+i**\ *zinc*..
    Yet another option is to specify **-C**\ *color1,color2*\ [,\ *color3*,...]
    to build a linear continuous cpt from those colors automatically.  
    In this case *color*\ **n** can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ).

.. _-G:

**-G**\ *fill*
    Sets color or shade for vector interiors [Default is no fill].

.. _-I:

**-I**\ [**x**]\ *dx*\ [/*dy*]
    Only plot vectors at nodes every *x\_inc*, *y\_inc* apart (must be
    multiples of original grid spacing). Append **m** for arc minutes or
    **s** for arc seconds.  Alternatively, use **-Ix** to specify the
    multiples *multx*\ [/*multy*] directly [Default plots every node]. 

.. _-N:

**-N**
    Do NOT clip vectors at map boundaries [Default will clip]. 

.. _-Q:

**-Q**\ *parameters*
    Modify vector parameters. For vector heads, append vector head
    *size* [Default is 0, i.e., stick-plot]. See VECTOR ATTRIBUTES for
    specifying additional attributes. 

.. _-R:

.. |Add_-R| replace:: Specify a subset of the grid.
.. include:: explain_-R.rst_

.. _-S:

**-S**\ [**i**\ \|\ **l**\ ]\ *scale*\ [*unit*]
    Sets scale for vector plot length in data units per plot distance measurement
    unit [1]. Append **c**, **i**, or **p** to indicate the measurement
    unit (cm, inch, or point); if no unit is given we use the default value that
    is controlled by :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>`. Alternatively,
    use **-Sl**\ *length*\ [*unit*] to set a fixed plot length for all vectors.  Vectors given
    via plot unit scaling will plot as straight vectors and their lengths are not
    affected by map projection and coordinate locations.
    For geographic data you may alternatively give *scale* in data units per
    map distance unit (see UNITS). Then, your user units are scaled to map distances in the given unit
    which are projected to plot dimensions.  These are geo-vectors that follow
    great circle paths and their lengths are affected by the map projection and their
    coordinates.  Finally, use **-Si** if it is simpler to give the reciprocal scale in
    measurement unit per data unit or km per data unit.  To report the minimum, maximum,
    and mean scaled vector length, use **-Vl**.

.. _-T:

**-T**
    Means the azimuths of Cartesian data sets should be adjusted according to the
    signs of the scales in the x- and y-directions [Leave alone].  This option can
    be used to convert vector azimuths in cases when a negative scale is used in
    one of both directions (e.g., positive down).

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *pen*
    Set pen attributes used for vector outlines [Default: width =
    default, color = black, style = solid]. 

.. _-X:

.. include:: explain_-XY.rst_

.. _-Z:

**-Z**
    The theta grid provided contains azimuths rather than directions (implies **-A**). 

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_distunits.rst_

.. include:: explain_grd_inout_short.rst_

.. include:: explain_vectors.rst_


Examples
--------

To draw the vector field given by the files r.nc and theta.nc on a
linear plot with scale 5 cm per data unit, using vector rather than
stick plot, scale vector magnitudes so that 10 units equal 1 inch, and
center vectors on the node locations, run

   ::

    gmt grdvector r.nc theta.nc -Jx5c -A -Q0.1i+e+jc -S10i -pdf gradient

To plot a geographic data sets given the files comp_x.nc and comp_y.nc,
using a length scale of 200 km per data unit and only plot every 3rd node in either direction, try

   ::

    gmt grdvector comp_x.nc comp_y.nc -Ix3 -JH0/20c -Q0.1i+e+jc -S200k -pdf globe

Notes
-----

Be aware that using **-I** may lead to aliasing unless
your grid is smoothly varying over the new length increments.
It is generally better to filter your grids and resample at a
larger grid increment and use these grids instead of the originals.

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`grdcontour`, :doc:`plot`
