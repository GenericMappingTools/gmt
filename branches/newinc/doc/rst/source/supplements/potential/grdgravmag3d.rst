.. index:: ! grdgravmag3d

************
grdgravmag3d
************

.. only:: not man

    grdgravmag3d - Compute the gravity effect of a grid by the method of Okabe

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**grdgravmag3d** *grdfile_top* [*grdfile_bot*] [ **-C**\ *density* ]
[ **-D** ] [ **-F**\ *xy_file* ]
[ **-G**\ *outgrid* ]
|SYN_OPT-I|
[ **-L**\ *z_obs* ]
[ **-Q**\ [\ **n**\ *n_pad*]\ \|\ [*pad_dist*]\ \|\ [<w/e/s/n>] ]
[ |SYN_OPT-V| ]
[ **-Z**\ *level* ]
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

*grdfile_top* [*grdfile_bot*]
   Grid file whose gravity efect is going to be computed. If two grids are
   provided then the gravity/magnetic efect of the volume between them is computed. 

**-C**\ *density*
    Sets body density in SI. This option is mutually exclusive with
    **-H**.
**-F**\ *xy\_file*
    Provide locations where the anomaly will be computed. Note this
    option is mutually exlusive with **-G**.
**-G**\ *outgrid*
    Output the gravity anomaly at nodes of this grid file.

.. include:: ../../explain_-I.rst_

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-R.rst_

Optional Arguments
------------------

**-L**\ *z_obs* 
    sets level of observation [Default = 0]. That is the height (z) at
    which anomalies are computed.

**-Q**\ [\ **n**\ *n_pad*]\ \|\ [\ *pad_dist*]\ \|\ [<w/e/s/n>]
    Extend the domain of computation with respect to output **-R** region. 
      **-Qn**\ *n_pad* artifficially extends the width of the outer rim of
      cells to have a fake width of *n_pad* * dx[/dy].

      **-Q**\ *pad_dist* extend the region by west-pad, east+pad, etc.

      **-Q**\ *region* Same sintax as **-R**.

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

**-Z**\ *level*
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
