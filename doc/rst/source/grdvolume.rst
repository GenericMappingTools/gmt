.. index:: ! grdvolume
.. include:: module_core_purpose.rst_

*********
grdvolume
*********

|grdvolume_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdvolume** *ingrid* [ |-C|\ *cval* or |-C|\ *low/high/delta* or |-C|\ **r**\ *low/high* or |-C|\ **r**\ *cval*]
[ |-D| ]
[ |-L|\ *base* ]
[ |SYN_OPT-R| ]
[ |-S|\ [*unit*] ]
[ |-T|\ [**c**\|\ **h**] ]
[ |SYN_OPT-V| ]
[ |-Z|\ *fact*\ [/*shift*] ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT--| ]


|No-spaces|

Description
-----------

**grdvolume** reads a 2-D grid file and calculates the volume contained below the surface and above the plane specified
by the given contour (or zero if not given) and reports the contour, area, volume, and maximum mean height (volume/area).
Alternatively, specify a range of contours to be tried and **grdvolume** will determine the volume and area inside
the contour for all contour values. Using |-T|, the contour that produced the maximum mean height (or maximum
curvature of heights vs contour value) is reported as well. This feature may be used with :doc:`grdfilter`
in designing an Optimal Robust Separator [*Wessel*, 1998; 2016].

Required Arguments
------------------

.. |Add_ingrid| replace:: 2-D gridded data file.
.. include:: explain_grd_inout.rst_
    :start-after: ingrid-syntax-begins
    :end-before: ingrid-syntax-ends

Optional Arguments
------------------

.. _-C:

**-C**\ [*cval*\|\ *low/high/delta*] or **-C**\ **r**\ [*cval*\|\ *low/high*]
    By default, report the area, volume, and mean height (where height = volume / area)
    of the entire grid. While the value in the first column will be 0, this does not
    mean that a zero-contour was traced (unless **-C**\ 0 is used). Alternatively, use
    one of the following to report a different statistic:

        * **-C**\ *cval* - Report the area, volume, and mean height above the
          *cval* contour and below the grid surface.

        * **-C**\ *low/high/delta* - Report the area, volume, and mean height
          above each contour from *low* to *high* in steps of *delta* and below
          the grid surface. The area is calculated in the plane of the contour.

        * **-C**\ **r**\ *cval* - Report the area, volume, and mean height
          below the *cval* contour and above the grid surface. **Note**: This
          defines an *outside* volume.

        * **-C**\ **r**\ *low/high* - Report the area, volume, and mean height
          between *low* and *high* and above the grid surface. For example, use
          this form to compute the volume of water between two contours.

.. _-D:

**-D**
    Requires **-C**\ *low/high/delta* and will compute the area and volume of each horizontal *slice* as defined by the
    contours.  The reported contour and area values refer to the base of the slice, and the *height* is set to *delta*
    (since that is the thickness of all slices).

.. _-L:

**-L**\ *base*
    Also add in the volume from the level of the contour down to *base* [Default base is contour].

.. _-S:

**-S**\ [*unit*]
    For geographical grids, append a unit from
    **e**\|\ **f**\|\ **k**\|\ **M**\|\ **n**\|\ **u**
    [Default is meter (**e**)].

.. _-T:

**-T**\ [**c**\|\ **h**]
    Determine the single contour that maximized the average height (=
    volume/area). Select **-Tc** to use the maximum curvature of heights
    versus contour value rather than the contour with the maximum height
    to pick the best contour value (requires |-C|).

.. |Add_-R| replace:: |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**\ *fact*\ [/*shift*]
    Optionally subtract *shift* before scaling data by *fact*. [Default
    is no scaling]. (Numbers in |-C|, |-L| refer to values after
    this scaling has occurred).

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_help.rst_

Examples
--------

.. include:: explain_example.rst_

To determine area (in km\ :sup:`2`), volume (in km\ :sup:`3`), and mean height (in km) of all land areas
(above zero contour) in the Hawaiian Islands from the remote grid @earth_relief_05m (height in m), use

::

  gmt grdvolume @earth_relief_05m -R190/210/15/25 -C0 -Sk -Z0.001

To find the volume below the surface peaks.nc and above the contour z = 250 m in meters, use

::

  gmt grdvolume peaks.nc -Se -C250

To search for the contour, ranging from 100 to 300 in steps of 10, that
maximizes the ratio of volume to surface area for the file peaks.nc, use

::

  gmt grdvolume peaks.nc -C100/300/10 -Th > results.d

To see the areas and volumes for all the contours in the previous example, use

::

  gmt grdvolume peaks.nc -C100/300/10 > results.d

To find the volume of water in a lake with its free surface at 0 and max depth of 300 meters, use

::

  gmt grdvolume lake.nc -Cr-300/0

Volume integration
------------------

The surface will be approximated using a bilinear expression for the *z*-value inside each grid box
defined by four grid nodes: :math:`z(x,y) = z_0 + z_x^{'}x + z_y^{'}y + z_{xy}^{''}xy`, where the
first term is the grid value at the lower left corner of the cell (where our relative coordinates
*x* = *y* = 0). The primed *z*-values are derivatives in *x*, *y*, and both directions, respectively.
We analytically integrate this expression within each box, allowing for straight line contour intersections
to go through a box and affect the integration domain and limits.

Notes
-----

#. The output of **grdvolume** is one or more records (one per contour if |-C|
   is set to search multiple contours) containing *contour area volume volume/area*.
   These records are written to standard output.
#. For geographical grids we convert degrees to "Flat Earth" distances in
   meter.  You can use |-S| to select another distance unit.  The
   area is then reported in this unit squared while the volume is reported
   in unit\ :sup:`2` * z_unit quantities.
#. **grdvolume** distinguishes between gridline and pixel-registered grids.
   In both cases the area and volume are computed up to the grid
   boundaries. That means that in the first case the grid cells on the
   boundary only contribute half their area (and volume), whereas in the
   second case all grid cells are fully used. The exception is when the
   |-C| flag is used: since contours do not extend beyond the outermost
   grid point, both grid types are treated the same. That means the outer
   rim in pixel oriented grids is ignored when using the |-C| flag.

See Also
--------

:doc:`gmt`,
:doc:`grdfilter`,
:doc:`grdmask`,
:doc:`grdmath`

References
----------

Wessel, P., 1998, An empirical method for optimal robust
regional-residual separation of geophysical data, *Math. Geol.*, **30**\ (4), 391-408,
https://doi.org/10.1023/A:1021744224009.

Wessel, P., 2016, Regional–residual separation of bathymetry and revised estimates of Hawaii plume flux,
*Geophys. J. Int.*, **204**\ (2), 932-947, https://doi.org/10.1093/gji/ggv472.
