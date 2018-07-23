.. index:: ! gravmag3d

*********
gravmag3d
*********

.. only:: not man

    gravmag3d - Compute the gravity/magnetic effect of a 3-D body by the method of Okabe

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt gravmag3d** |-T|\ **p**\ *xyz_file*\ [**+m**] |-T|\ **v**\ *vert_file* OR |-T|\ **r\|s**\ *raw_file*
[ |-C|\ *density* ]
[ |-D| ]
[ |-E|\ *thickness* ]
[ |-F|\ *xy_file* ]
[ |-G|\ *outputgrid* ]
[ |-H|\ *f_dec*/*f_dip*/*m_int*/*m_dec*/*m_dip* ]
[ |-L|\ *z_observation* ]
[ |-S|\ *radius* ]
[ |-Z|\ *level* ]
[ |SYN_OPT-V| ]
[ **-fg**\ ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**gravmag3d** will compute the gravity or magnetic anomaly of a body
described by a set of triangles. The output can either be along a given
set of xy locations or on a grid. This method is not particularly fast
but allows computing the anomaly of arbitrarily complex shapes.

Required Arguments
------------------

.. _-C:

**-C**\ *density*
    Sets body density in SI. This option is mutually exclusive with **-H**.

.. _-H:

**-H**\ *f_dec*/*f_dip*/*m_int*/*m_dec*/*m_dip*
    Sets parameters for computing a magnetic anomaly. Use
    *f_dec*/*f_dip* to set the geomagnetic declination/inclination in
    degrees. *m_int*/*m_dec*/*m_dip* are the body magnetic intensity
    declination and inclination.

.. _-F:

**-F**\ *xy_file*
    Provide locations where the anomaly will be computed. Note this
    option is mutually exclusive with **-G**.

.. _-G:

**-G**\ *outgrid*
    Output the gravity or magnetic anomaly at nodes of this grid file.

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-R.rst_

.. _-T:

**-Tp**\ *xyz_file*\ [**+m**] **-Tv**\ *vert_file* OR **Tr\|s**\ *raw_file*
    Gives names of xyz (**-Tp**\ *xyz_file*\ [**+m**]) and vertex (**-Tv**\ *vert_file*) files defining a close surface.
    The file formats correspond to the output of the :doc:`triangulate </triangulate>` program.
    The optional **+m** flag to **-Tp** instructs the program that the xyzm file
    has four columns and that the fourth column contains the magnetization intensity (plus signal),
    which needs not to be constant. In this case the third argument of the **-H** option is
    ignored. A *raw* format (selected by the **-Tr** option) is a file with N rows (one per triangle)
    and 9 columns corresponding to the x,y,x coordinates of each of the three vertex of each triangle.
    Alternatively, the **-Ts** option indicates that the surface file is in the ASCII STL (Stereo Lithographic) format.
    These two type of files are used to provide a closed surface.

Optional Arguments
------------------

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-E:

**-E**\ [*thickness*]
    give layer thickness in m [Default = 0 m]. Use this option only when
    the triangles describe a non-closed surface and you want the anomaly
    of a constant thickness layer.

.. _-L:

**-L**\ [*z_observation*]
    sets level of observation [Default = 0]. That is the height (z) at
    which anomalies are computed.

.. _-S:

**-S**\ *radius*
    search radius in km. Triangle centroids that are further away than
    *radius* from current output point will not be taken into account.
    Use this option to speed up computation at expenses of a less
    accurate result.

.. _-Z:

**-Z**\ [*level*]
    level of reference plane [Default = 0]. Use this option when the
    triangles describe a non-closed surface and the volume is defined
    from each triangle and this reference level. An example will be the
    hater depth to compute a Bouguer anomaly.

**-fg**
   Geographic grids (dimensions of longitude, latitude) will be converted to
   meters via a "Flat Earth" approximation using the current ellipsoid parameters.

.. include:: ../../explain_help.rst_

Grid Distance Units
-------------------

If the grid does not have meter as the horizontal unit, append **+u**\ *unit* to the input file name to convert from the
specified unit to meter.  If your grid is geographic, convert distances to meters by supplying **-fg** instead.

Examples
--------

Suppose you ...

   ::

    gmt gravmag3d ...

See Also
--------

:doc:`gmt </gmt>`, :doc:`grdgravmag3d`,
:doc:`talwani2d </supplements/potential/talwani2d>`,
:doc:`talwani3d </supplements/potential/talwani3d>`

Reference
---------

Okabe, M., Analytical expressions for gravity anomalies due to
polyhedral bodies and translation into magnetic anomalies, *Geophysics*,
44, (1979), p 730-741.
