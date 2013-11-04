.. index:: ! grdgravmag3d

************
grdgravmag3d
************

.. only:: not man

    grdgravmag3d - Compute the gravity effect of a grid by the method of Okabe

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**grdgravmag3d** [ **-C**\ *density* ] [ **-D** ] [ **-F**\ *xy_file* ]
[ **-G**\ *outputgrid.nc* ] [ **-L**\ *z_observation* ] [ **-M** ]
|SYN_OPT-V|
[ **-Z**\ *level* ]
[ |SYN_OPT-V| ]
[ **-fg** ]

|No-spaces|

Description
-----------

**grdgravmag3d** will compute the gravity anomaly of a body described by
one or (optionally) two grids The output can either be along a given set
of xy locations or on a grid. This method is not particularly fast but
allows computing the anomaly of arbitrarily complex shapes.

Required Arguments
------------------

**-C**\ *density*
    Sets body density in SI. This option is mutually exclusive with
    **-H**.
**-F**\ *xy\_file*
    Provide locations where the anomaly will be computed. Note this
    option is mutually exlusive with **-G**.
**-G**\ *outgrid.nc*
    Output the gravity anomaly at nodes of this grid file.

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-R.rst_

Optional Arguments
------------------

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

**-L**
    sets level of observation [Default = 0]. That is the height (z) at
    which anomalies are computed.
**-Z**
    level of reference plane [Default = 0]. Use this option when the
    triangles describe a non-closed surface and the volume is deffined
    from each triangle and this reference level. An example will be the
    whater depth to compute a Bouguer anomaly.

**-fg**
   Geographic grids (dimensions of longitude, latitude) will be converted to
   meters via a "Flat Earth" approximation using the current ellipsoid parameters.

.. include:: ../../explain_help.rst_

Grid Distance Units
-------------------

If the grid does not have meter as the horizontal unit, append **+u**\ *unit* to the input file name to convert from the
specified unit to meter. If your grid is geographic, convert distances to meters by supplying **-fg** instead.

Examples
--------

Suppose you want to compute the gravity effect of the phantom "Sandy
Island" together with its not phantom seamount


   ::

    gmt grdgravmag3d sandy_bat.grd -C1700 -Z-4300 -M -I1m -Gsandy_okb.grd -V

See Also
--------

:doc:`gmt </gmt>`, :doc:`gmtgravmag3d`

Reference
---------

Okabe, M., Analytical expressions for gravity anomalies due to
polyhedral bodies and translation into magnetic anomalies, *Geophysics*,
44, (1979), p 730-741.
