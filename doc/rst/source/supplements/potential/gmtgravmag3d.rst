.. index:: ! gmtgravmag3d
.. include:: ../module_supplements_purpose.rst_

************
gmtgravmag3d
************

|gmtgravmag3d_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt gravmag3d** *xyz_file* |-T|\ **v**\ *vert_file* OR |-T|\ **r\|s**\ *raw_file* OR |-M|\ **+s**\ *body,params*
[ |-C|\ *density* ]
[ |-E|\ *thickness* ]
[ |-F|\ *xy_file* ]
[ |-G|\ *outputgrid* ]
[ |-H|\ *f_dec*/*f_dip*/*m_int*/*m_dec*/*m_dip* ]
[ |-L|\ *z_observation* ]
[ |-S|\ *radius* ]
[ |-Z|\ *level* ]
[ |SYN_OPT-V| ]
[ **-fg**]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**gravmag3d** will compute the gravity or magnetic anomaly of a body described by a set of triangles.
The output can either be along a given set of xy locations or on a grid. This method is not particularly
fast but allows computing the anomaly of arbitrarily complex shapes.

Required Arguments (not all)
----------------------------

.. _-C:

**-C**\ *density*
    Sets body density in SI. This option is mutually exclusive with **-H**.

.. _-H:

**-H**\ *f_dec*/*f_dip*/*m_int*/*m_dec*/*m_dip*
    Sets parameters for computing a magnetic anomaly. Use *f_dec*/*f_dip* to set the geomagnetic
    declination/inclination in degrees. *m_int*/*m_dec*/*m_dip* are the body magnetic intensity
    declination and inclination.

.. _-F:

**-F**\ *xy_file*
    Provide locations where the anomaly will be computed. Note this
    option is mutually exclusive with **-G**.

.. _-G:

**-G**\ *outgrid*
    Output the gravity or magnetic anomaly at nodes of this grid file.

.. _-M:

**-M+s**\ *body,params* (An alaternative to **-Tr**\ /**-Ts**). Create geometric bodies and compute their grav/mag effect.
    Select among one or more of the following bodies, where *x0* & *y0* represent the horizontal coordinates
    of the body center [default to 0,0 positive up], *npts* is the number of points that a circle is discretized
    and *n_slices* apply when bodies are made by a pile of slices. For example Spheres and Ellipsoids are made of
    *2 x n_slices* and Bells have *n_slices* [Default 5]. It is even possible to select more than one body. For example
    **-M+s**\ *prism,1/1/1/-5/-10/1*\ **+s**\ *sphere,1/-5* computes the effect of a prism and a sphere. Unfortunately there is
    no current way of selecting distinct densities or magnetic parameters for each body.

      - *bell,height/sx/sy/z0[/x0/y0/n_sig/npts/n_slices]* Gaussian of height *height* with characteristic STDs *sx* and *sy*. The base width (at depth *z0*) is controlled by the number of sigmas (*n_sig*) [Default = 2]

      - *cylinder,rad/height/z0[/x0/y0/npts/n_slices]* Cylinder of radius *rad* and height *height* and base at depth *z0*

      - *cone,semi_x/semi_y/height/z0[/x0/y0/npts]* Cone of semi axes *semi_x/semi_y* height *height* and base at depth *z0*

      - *ellipsoid,semi_x/semi_y/semi_z/z_center[/x0/y0/npts/n_slices]* Ellipsoid of semi axes *semi_x/semi_y/semi_z* and center depth *z_center*

      - *prism,side_x/side_y/side_z/z0[/x0/y0]* Prism of sides *x/y/z* and base at depth *z0*

      - *pyramid,side_x/side_y/height/z0[/x0/y0]* Pyramid of sides *x/y* height *height* and base at depth *z0*

      - *sphere,rad/z_center[/x0/y0/npts/n_slices]* Sphere of radius *rad* and center at depth *z_center*

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-R.rst_

.. _-T:

**-Tv**\ *vert_file* (must have when passing a *xyz_file*) OR **-Tr\|s**\ *raw_file*
    Gives names of a xyz and vertex (**-Tv**\ *vert_file*) files defining a close surface.
    The file formats correspond to the output of the :doc:`triangulate </triangulate>` program.
    The *xyz* file can have 3, 4, 5, 6 or 8 columns. In first case (3 columns) the magnetization (or density) are
    assumed constant (controlled by **-C** or **-H**). Following cases are: 4 columns -> 4rth col magnetization intensity;
    5 columns: mag, mag dip; 6 columns: mag, mag dec, mag dip; 8 columns: field dec, field dip, mag, mag dec, mag dip.
    When n columns > 3 the third argument of the **-H** option is ignored. A *raw* format (selected by the **-Tr** option)
    is a file with N rows (one per triangle) and 9 columns corresponding to the x,y,x coordinates of each of the three
    vertex of each triangle. Alternatively, the **-Ts** option indicates that the surface file is in the ASCII STL
    (Stereo Lithographic) format. These two type of files are used to provide a closed surface.

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

To compute the magnetic anomaly of a cube of unit sides located at 5 meters depth and centered at -10,1 in a domain
*-R-15/15/-15/15* with a magnetization of 10 Am with a declination of 10 degrees, inclination of 60 in a magnetic field
with -10 deg of declination and 40 deg of inclination, do::

    gmt gmtgravmag3d -R-15/15/-15/15 -I1 -H10/60/10/-10/40 -M+sprism,1/1/1/-5/-10/1 -Gcube_mag_anom.grd

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
