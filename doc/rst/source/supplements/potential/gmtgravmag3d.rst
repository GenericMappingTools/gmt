************
gmtgravmag3d
************

gmtgravmag3d - Compute the gravity/magnetic effect of a body by the
method of Okabe

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmtgravmag3d** [ **-C**\ *density* ] [ **-D** ] [ **-E**\ *thickness*
] [ **-F**\ *xy\_file* ] [ **-G**\ *outputgrid.nc* ] [
**-H**\ *f\_dec*/*f\_dip*/*m\_int*/*m\_dec*/*m\_dip* ] [
**-L**\ *z\_observation* ]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [ **-S**\ *radius* ] [
**-T**\ [[*d*\ ]\ *xyz\_file*/*vert\_file*\ [*/m*\ ]]\|[*r\|s*\ ]\ *raw\_file*
] [ **-Z**\ *level* ] [ **-V**\ [*level*\ ] ] [
**-fg**\ ]

Description
-----------

**gmtgravmag3d** will compute the gravity or magnetic anomaly of a body
described by a set of triangles. The output can either be along a given
set of xy locations or on a grid. This method is not particularly fast
but allows computing the anomaly of arbitrarily complex shapes.

.. include:: ../../explain_commonitems.rst_

Required Arguments
------------------

**-C**\ *density*
    Sets body density in SI. This option is mutually exclusive with
    **-H**.
**-H**\ *f\_dec*/*f\_dip*/*m\_int*/*m\_dec*/*m\_dip*
    Sets parameters for computing a magnetic anomally. Use
    *f\_dec*/*f\_dip* to set the geomagnetic declination/inclination in
    degrees. *m\_int*/*m\_dec*/*m\_dip* are the body magnetic intensity
    declination and inclination.
**-F**\ *xy\_file*
    Provide locations where the anomaly will be computed. Note this
    option is mutually exlusive with **-G**.
**-G**\ *outgrid.nc*
    Output the gravity or magnetic anomaly at nodes of this grid file.

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-R.rst_

Optional Arguments
------------------

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

**-E**\ [*thickness*]
    give layer thickness in m [Default = 0 m]. Use this option only when
    the triangles describe a non-closed surface and you want the anomaly
    of a constant thickness layer.
**-L**\ [*z\_observation*]
    sets level of observation [Default = 0]. That is the height (z) at
    which anomalies are computed.
**-S**\ *radius*
    search radius in km. Triangle centroids that are further away than
    *radius* from current output point will not be taken into account.
    Use this option to speed up computation at expenses of a less
    accurate result.
**-T**\ [[*d*\ ]\ *xyz\_file*/*vert\_file*\ [*/m*\ ]]\|[*r\|s*\ ]\ *raw\_file*]
    Give either names of xyz[m] and *vertex* files or of a *raw* or
    *stl* file defining a close surface. In the first case append a *d*
    imediatly after **-T** and optionaly a */m* after the vertex file
    name. In the second case append a *r* or a *s* imediatly after
    **-T** and before the file name. A *vertex* file is a file with N
    rows (one per triangle) and 3 columns with integers defining the
    order by which the points in the *xyz* file are to be connected to
    form a triangle. The output of the program triangulate comes in this
    format. The optional */m* instructs the program that the xyzm file
    has four columns and that the fourth column contains the
    magnetization intensity (plus signal), which needs not to be
    constant. In this case the third argument of the **-H** option is
    ignored. A *raw* format (selected by the ’r’ flag is a file with N
    rows (one per triangle) and 9 columns corresponding to the x,y,x
    coordinates of each of the three vertex of each triangle.
    Alternatively, the ’s’ flag indicates that the surface file is in
    the ascii STL (Stereo Lithographic) format. These two type of files
    are used to provide a closed surface.
**-Z**\ [*level*\ ]
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
specified unit to meter.  If your grid is geographic, convert distances to meters by supplying **-fg** instead.

Examples
--------

Suppose you ...

**gmtgravmag3d**

See Also
--------

`GMT <../../GMT.html>`_, `grdgravmag3d <grdgravmag3d.html>`_

Reference
---------

Okabe, M., Analytical expressions for gravity anomalies due to
polyhedral bodies and translation into magnetic anomalies, *Geophysics*,
44, (1979), p 730-741.
