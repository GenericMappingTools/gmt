.. index:: ! img2google

**********
img2google
**********

.. only:: not man

img2google - Create Google Earth KML overlay tiles from bathymetry
Mercator img grid

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**img2google**
|SYN_OPT-R|
[ *imgfile* ]
[ |-A|\ *mode*\ [*altitude*] ]
[ |-C| ]
[ |-F|\ *fademin/fademax* ]
[ |-G|\ *prefix* ]
[ |-L|\ *LODmin/LODmax* ]
[ |-N|\ *layername* ]
[ |-T|\ *doctitle* ]
[ |-U|\ *URL* ]
[ |SYN_OPT-V| ]
[ |-Z| ]
[ |SYN_OPT--| ]

Description
-----------

**img2google** is a shell script that reads a 1x1 minute Mercator surface relief img file and
creates a Google Earth overlay KML file and associated PNG tile for the
specified region. If no input file is given we use topo.18.1.img.

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

Options
-------

*imgfile*
    An img format bathymetry/topography file such as those created by
    Sandwell and Smith. If this files does not exist in the current
    directory and the user has set the environment variable
    **$GMT_DATADIR**, then :doc:`img2grd` will try
    to find *imgfile* in **$GMT_DATADIR**.

.. _-A:

**-A**
    Selects one of 5 altitude modes recognized by Google Earth that
    determines the altitude (in m) of the image: **G** clamped to the
    ground, **g** append altitude relative to ground, **a** append absolute
    altitude, **s** append altitude relative to seafloor, and **S** clamp it
    to the seafloor [Default].

.. _-C:

**-C**
    Turn on clipping so that only portions below sea level will be visible
    in the image [no clipping].

.. _-F:

**-F**
    Sets the distance over which the geometry fades, from fully opaque to
    fully transparent. These ramp values, expressed in screen pixels, are
    applied at the minimum and maximum end of the LOD (visibility) limits,
    respectively. [no fading (0/0)].

.. _-G:

**-G**
    Specify the prefix for the output image file (the extensions are set
    automatically). Default uses the naming
    topoN\|S\ *<north>*\ E\|W<*west*\ >.

.. _-L:

**-L**
    Measurement in screen pixels that represents the minimum limit of the
    visibility range for a given Region Google Earth calculates the size of
    the Region when projected onto screen space. Then it computes the square
    root of the Region's area (if, for example, the Region is square and the
    viewpoint is directly above the Region, and the Region is not tilted,
    this measurement is equal to the width of the projected Region). If this
    measurement falls within the limits defined by *LODmin* and *LODmax*
    (and if the region is in view), the Region is active. If this limit is
    not reached, the associated geometry is considered to be too far from
    the user's viewpoint to be drawn. *LODmax* represents the maximum limit
    of the visibility range for a given Region. A value of 1, the default,
    indicates "active to infinite size." [always active].

.. _-N:

**-N**
    Append the layername of the image (use quotes if strings contain spaces)
    [topoN\|S<*north*>\ E\|W<*west*>].

.. _-T:

**-T**
    Append the document title (use quotes if strings contain spaces)
    ["Predicted bathymetry"].

.. _-U:

**-U**
    By default, images are referenced locally relative to the KML file.
    Specify an URL to prepend a server address to the image name reference
    [local].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-Z:

**-Z**
    Uses zip (which must be installed) to create a \*.kmz file for easy
    distribution; append **+** to delete the KML and PNG file after zipping
    [No zipping].

Examples
--------

To create a 10x10 degree Google Earth KML tile for the region
**-R**\ 170/180/20/30 using the default *topo.18.1.img* and output
naming convention, try

   ::

    img2google -R170/180/20/30

To make the same tile with a previous file such as *topo.15.1.img*, run in verbose
mode, clip so only oceanic areas are visible, name the output oldimage,
specify the KML metadata directly (including setting the image altitude
to 10 km), and make a single \*.kmz file, try

   ::

    img2google topo.15.1.img -R170/180/20/30 -Aa10000 -C -Goldimage \
    -N"My KML title" -T"My KML title" -Uhttp://my.server.com/images -V -Z

DATA SETS
---------

For topo.18.1.img and other Sandwell/Smith altimetry-derived Mercator
grids, visit http://topex.ucsd.edu.

SEE ALSO
--------

:doc:`gmt </gmt>`
:doc:`img2grd`,
:doc:`psconvert </psconvert>`
