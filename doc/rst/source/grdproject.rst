.. index:: ! grdproject

**********
grdproject
**********

.. only:: not man

    grdproject - Forward and inverse map transformation of grids

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdproject** *in_grdfile* |-G|\ *out_grdfile* |-J|\ *parameters*
[ |-C|\ [*dx/dy*] ]
[ |-D|\ *xinc*\ [*unit*][\ **+e**\ \|\ **n**][/\ *yinc*\ [*unit*][\ **+e**\ \|\ **n**]] ]
[ |-E|\ *dpi* ] [ |-F|\ [**c**\ \|\ **i**\ \|\ **p**\ \|\ **e**\ \|\ **f**\ \|\ **k**\ \|\ **M**\ \|\ **n**\ \|\ **u**\ ] ] [ |-I| ] [ |-M|\ **c**\ \|\ **i**\ \|\ **p** ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdproject** will do one of two things depending whether **-I** has
been set. If set, it will transform a gridded data set from a
rectangular coordinate system onto a geographical system by resampling
the surface at the new nodes. If not set, it will project a geographical
gridded data set onto a rectangular grid. To obtain the value at each
new node, its location is inversely projected back onto the input grid
after which a value is interpolated between the surrounding input grid
values. By default bi-cubic interpolation is used. Aliasing is avoided
by also forward projecting the input grid nodes. If two or more nodes
are projected onto the same new node, their average will dominate in the
calculation of the new node value. Interpolation and aliasing is
controlled with the **-n** option. The new node spacing may be
determined in one of several ways by specifying the grid spacing, number
of nodes, or resolution. Nodes not constrained by input data are set to
NaN.

The **-R** option can be used to select a map region larger or smaller
than that implied by the extent of the grid file. 

Required Arguments
------------------

*in_grdfile*
    2-D binary grid file to be transformed. (See GRID FILE FORMATS below.)

.. _-G:

**-G**\ *out_grdfile*
    Specify the name of the output grid file. (See GRID FILE FORMATS below.) 

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

Optional Arguments
------------------

.. _-C:

**-C**\ [*dx/dy*\ ]
    Let projected coordinates be relative to projection center [Default
    is relative to lower left corner]. Optionally, add offsets in the
    projected units to be added (or subtracted when **-I** is set) to
    (from) the projected coordinates, such as false eastings and
    northings for particular projection zones [0/0].

.. _-D:

**-D**\ *xinc*\ [*unit*\ ][\ **+e**\ \|\ **n**][/\ *yinc*\ [*unit*\ ][\ **+e**\ \|\ **n**]]
    Set the grid spacing for the new grid. Append **m** for arc minute,
    **s** for arc second. If neither **-D** nor **-E** are set then we
    select the same number of output nodes as there are input nodes.

.. _-E:

**-E**\ *dpi*
    Set the resolution for the new grid in dots per inch.

.. _-F:

**-F**\ [**c**\ \|\ **i**\ \|\ **p**\ \|\ **e**\ \|\ **f**\ \|\ **k**\ \|\ **M**\ \|\ **n**\ \|\ **u**\ ]
    Force 1:1 scaling, i.e., output (or input, see **-I**) data are in
    actual projected meters [**e**\ ]. To specify other units, append
    **f** (foot), **k** (km), **M** (statute mile), **n** (nautical
    mile), **u** (US survey foot), **i** (inch), **c** (cm), or **p**
    (point). Without **-F**, the output (or input, see **-I**) are in
    the units specified by :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>` (but see **-M**).

.. _-I:

**-I**
    Do the Inverse transformation, from rectangular to geographical.

.. _-M:

**-Mc**\ \|\ **i**\ \|\ **p**
    Append **c**, **i**, or **p** to indicate that cm, inch, or point
    should be the projected measure unit [Default is set by
    :ref:`PROJ_LENGTH_UNIT <PROJ_LENGTH_UNIT>` in :doc:`gmt.conf`]. Cannot be used with **-F**.

.. _-R:

.. |Add_-R| replace:: You may ask to project only
    a subset of the grid by specifying a smaller input *w/e/s/n* region
    [Default is the region given by the grid file]. 
.. include:: explain_-R.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_-n.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout_short.rst_

Examples
--------

To transform the geographical grid dbdb5.nc onto a pixel Mercator grid at 300 dpi, run

   ::

    gmt grdproject dbdb5.nc -R20/50/12/25 -Jm0.25i -E300 -r -Gdbdb5_merc.nc

To inversely transform the file topo_tm.nc back onto a geographical grid, use

   ::

    gmt grdproject topo_tm.nc -R-80/-70/20/40 -Jt-75/1:500000 -I -D5m -V -Gtopo.nc

This assumes, of course, that the coordinates in topo_tm.nc were
created with the same projection parameters.

To inversely transform the file topo_utm.nc (which is in UTM meters)
back to a geographical grid we specify a one-to-one mapping with meter
as the measure unit:

   ::

    gmt grdproject topo_utm.nc -R203/205/60/65 -Ju5/1:1 -I -Mm -Gtopo.nc -V

To inversely transform the file data.nc (which is in Mercator meters with Greenwich
as the central longitude and a false easting of -4 and produced on the ellipse WGS-72)
back to a geographical grid we specify a one-to-one mapping with meter
as the measure unit:

   ::

    gmt grdproject data.nc -Jm/1:1 -I -F -C-4/0 -Gdata_geo.nc -V --PROJ_ELLIPSOID=WGS-72

Restrictions
------------

The boundaries of a projected (rectangular) data set will not
necessarily give rectangular geographical boundaries (Mercator is one
exception). In those cases some nodes may be unconstrained (set to NaN).
To get a full grid back, your input grid may have to cover a larger area
than you are interested in.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`, :doc:`mapproject`
