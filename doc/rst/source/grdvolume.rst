.. index:: ! grdvolume

*********
grdvolume
*********

.. only:: not man

    Calculate grid volume and area constrained by a contour

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdvolume** *grdfile* [ |-C|\ *cval* or |-C|\ *low/high/delta* or |-C|\ **r**\ *low/high* or |-C|\ **r**\ *cval*]
[ |-L|\ *base* ]
[ |SYN_OPT-R| ]
[ |-S|\ [*unit*] ] [ |-T|\ [**c**\ \|\ **h**] ]
[ |SYN_OPT-V| ]
[ |-Z|\ *fact*\ [/*shift*] ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT--| ]


|No-spaces|

Description
-----------

**grdvolume** reads a 2-D grid file and calculates the volume contained between the surface and the plane specified
by the given contour (or zero if not given) and reports the area, volume, and maximum mean height (volume/area).
Alternatively, specify a range of contours to be tried and **grdvolume** will determine the volume and area inside
the contour for all contour values. Using **-T**, the contour that produced the maximum mean height (or maximum
curvature of heights vs contour value) is reported as well. This feature may be used with :doc:`grdfilter`
in designing an Optimal Robust Separator [*Wessel*, 1998]. 

Required Arguments
------------------

*grdfile*
    The name of the input 2-D binary grid file. (See GRID FILE FORMAT below.)

Optional Arguments
------------------

.. _-C:

**-C**\ *cval* or **-C**\ *low/high/delta* or **-Cr**\ *low/high* or **-Cr**\ *cval*
    find area, volume and mean height (volume/area) inside the *cval* contour. Alternatively, search using
    all contours from *low* to *high* in steps of *delta*. [Default returns area, volume and mean height
    of the entire grid]. The area is measured in the plane of the contour. The **Cr** form on the other
    hand computes the volume between the grid surface and the plans defined by *low* and *high*,
    or below *cval* and grid's minimum. Note that this is an *outside* volume whilst the other forms
    compute an *inside* (below the surface) area volume. Use this form to compute for example the volume
    of water between two contours.

.. _-L:

**-L**\ *base*
    Also add in the volume from the level of the contour down to *base* [Default base is contour].

.. _-S:

**-S**\ [*unit*]
    For geographical grids, append a unit from
    **e**\ \|\ **f**\ \|\ **k**\ \|\ **M**\ \|\ **n**\ \|\ **u**
    [Default is meter (**e**)].

.. _-T:

**-T**\ [**c**\ \|\ **h**]
    Determine the single contour that maximized the average height (=
    volume/area). Select **-Tc** to use the maximum curvature of heights
    versus contour value rather than the contour with the maximum height
    to pick the best contour value (requires **-C**). 

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-Z:

**-Z**\ *fact*\ [/*shift*]
    Optionally subtract *shift* before scaling data by *fact*. [Default
    is no scaling]. (Numbers in **-C**, **-L** refer to values after
    this scaling has occurred). 

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout_short.rst_

Examples
--------

To determine the volume in km^3 under the surface hawaii_topo.nc (height in km), use

   ::

    gmt grdvolume hawaii_topo.nc -Sk

To find the volume between the surface peaks.nc and the contour z = 250 m in meters, use

   ::

    gmt grdvolume peaks.nc -Se -C250

To search for the contour, between 100 and 300 in steps of 10, that
maximizes the ratio of volume to surface area for the file peaks.nc, use

   ::

    gmt grdvolume peaks.nc -C0/300/10 -Th > results.d

To see the areas and volumes for all the contours in the previous example, use

   ::

    gmt grdvolume peaks.nc -C100/300/10 > results.d

To find the volume of water in a lake with its free surface at 0 and max depth of 300 meters, use

   ::

    gmt grdvolume lake.nc -Cr-300/0

Notes
-----

#. For geographical grids we convert degrees to "Flat Earth" distances in
   meter.  You can use **-S** to select another distance unit.  The
   area is then reported in this unit squared while the volume is reported
   in unit^2 * z_unit quantities.
#. **grdvolume** distinguishes between gridline and pixel-registered grids.
   In both cases the area and volume are computed up to the grid
   boundaries. That means that in the first case the grid cells on the
   boundary only contribute half their area (and volume), whereas in the
   second case all grid cells are fully used. The exception is when the
   **-C** flag is used: since contours do not extend beyond the outermost
   grid point, both grid types are treated the same. That means the outer
   rim in pixel oriented grids is ignored when using the **-C** flag.

See Also
--------

:doc:`gmt`,
:doc:`grdfilter`,
:doc:`grdmask`,
:doc:`grdmath`

References
----------

Wessel, P., 1998, An empirical method for optimal robust
regional-residual separation of geophysical data, *Math. Geol.*, **30**\ (4), 391-408.
