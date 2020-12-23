.. index:: ! grdgradient
.. include:: module_core_purpose.rst_

***********
grdgradient
***********

|grdgradient_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdgradient** *ingrid* |-G|\ *outgrid*
[ |-A|\ *azim*\ [/*azim2*] ] [ |-D|\ [**a**][**c**][**o**][**n**] ]
[ |-E|\ [**m**\|\ **s**\|\ **p**]\ *azim/elev*\ [**+a**\ *ambient*][**+d**\ *diffuse*][**+p**\ *specular*][**+s**\ *shine*] ]
[ |-N|\ [**e**\|\ **t**][*amp*][**+a**\ *ambient*][**+s**\ *sigma*][**+o**\ *offset*] ]
[ |-Q|\ **c**\|\ **r**\|\ **R** ]
[ |SYN_OPT-R| ] [ |-S|\ *slopefile* ]
[ |SYN_OPT-V| ] [ |SYN_OPT-f| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdgradient** may be used to compute the directional derivative in a
given direction (**-A**), or to find the direction (**-S**) [and the magnitude
(**-D**)] of the vector gradient of the data.

Estimated values in the first/last row/column of output depend on
boundary conditions (see **-n**).

Required Arguments
------------------

*ingrid*
    2-D grid file from which to compute directional derivative. (See GRID FILE FORMATS below).

.. _-G:

**-G**\ *outgrid*
    Name of the output grid file for the directional derivative. (See
    GRID FILE FORMATS below).

Optional Arguments
------------------

.. _-A:

**-A**\ *azim*\ [/*azim2*]
    Azimuthal direction for a directional derivative; *azim* is the
    angle in the x,y plane measured in degrees positive clockwise from
    north (the +y direction) toward east (the +x direction). The
    negative of the directional derivative, -[dz/dx\*sin(*azim*) +
    dz/dy\*cos(\ *azim*)], is found; negation yields positive values
    when the slope of z(x,y) is downhill in the *azim* direction, the
    correct sense for shading the illumination of an image (see
    :doc:`grdimage` and :doc:`grdview`) by a light source above the x,y plane
    shining from the *azim* direction. Optionally, supply two azimuths,
    **-A**\ *azim*/*azim2*, in which case the gradients in each of these
    directions are calculated and the one larger in magnitude is
    retained; this is useful for illuminating data with two directions
    of lineated structures, e.g., **-A**\ *0*/*270* illuminates from the
    north (top) and west (left).  Finally, if *azim* is a file it must
    be a grid of the same domain, spacing and registration as *ingrid*
    and we will update the azimuth at each output node when computing the
    directional derivatives.

.. _-D:

**-D**\ [**a**][**c**][**o**][**n**]
    Find the direction of the positive (up-slope) gradient of the data.
    To instead find the aspect (the down-slope direction), use **-Da**.
    By default, directions are measured clockwise from north, as *azim* in **-A**
    above. Append **c** to use conventional Cartesian angles measured
    counterclockwise from the positive x (east) direction. Append **o**
    to report orientations (0-180) rather than directions (0-360).
    Append **n** to add 90 degrees to all angles (e.g., to give
    local strikes of the surface ).

.. _-E:

**-E**\ [**m**\|\ **s**\|\ **p**]\ *azim/elev*\ [**+a**\ *ambient*][**+d**\ *diffuse*][**+p**\ *specular*][**+s**\ *shine*]
    Compute Lambertian radiance appropriate to use with :doc:`grdimage` and :doc:`grdview`.
    The Lambertian Reflection assumes an ideal surface that
    reflects all the light that strikes it and the surface appears
    equally bright from all viewing directions. Here, *azim* and *elev* are
    the azimuth and elevation of the light vector. Optionally, supply
    *ambient* [0.55], *diffuse* [0.6], *specular* [0.4], or *shine* [10],
    which are parameters that control the reflectance properties of the
    surface. Default values are given in the brackets. Use **-Es** for a
    simpler Lambertian algorithm. Note that with this form you only have
    to provide azimuth and elevation. Alternatively, use **-Ep** for
    the Peucker piecewise linear approximation (simpler but faster
    algorithm; in this case the *azim* and *elev* are hardwired to 315
    and 45 degrees. This means that even if you provide other values
    they will be ignored.)

.. _-N:

**-N**\ [**e**\|\ **t**][*amp*][**+a**\ *ambient*][**+s**\ *sigma*][**+o**\ *offset*]
    Normalization. [Default is no normalization.] The actual gradients *g*
    are offset and scaled to produce normalized gradients *gn* with a
    maximum output magnitude of *amp*. If *amp* is not given, default
    *amp* = 1. If *offset* is not given, it is set to the average of
    *g*. **-N** yields *gn* = *amp* \* (*g* - *offset*)/max(abs(\ *g* -
    *offset*)). **-Ne** normalizes using a cumulative Laplace
    distribution yielding *gn* = *amp* \* (1.0 -
    exp(sqrt(2) \* (*g* - *offset*)/ *sigma*)), where
    *sigma* is estimated using the L1 norm of (*g* - *offset*) if it is
    not given. **-Nt** normalizes using a cumulative Cauchy distribution
    yielding *gn* = (2 \* *amp* / PI) \* atan( (*g* - *offset*)/
    *sigma*) where *sigma* is estimated using the L2 norm of (*g* -
    *offset*) if it is not given. To use *offset* and/or *sigma* from a
    previous calculation, leave out the argument to the modifier(s) and
    see **-Q** for usage.  As a final option, you may add **+a**\ *ambient*
    to add *ambient* to all nodes after gradient calculations are completed.

.. _-Q:

**-Qc**\|\ **r**\|\ **R**
    Controls how normalization via **-N** is carried out.  When multiple grids
    should be normalized the same way (i.e., with the same *offset* and/or *sigma*),
    we must pass these values via **-N**.  However, this is inconvenient if we
    compute these values from a grid.  Use **-Qc** to save the results of
    *offset* and *sigma* to a statistics file; if grid output is not needed
    for this run then do not specify **-G**. For subsequent runs, just use
    **-Qr** to read these values.  Using **-QR** will read then delete the
    statistics file. See TILES for more information.

.. _-R:

.. |Add_-R| replace:: Using the **-R** option
    will select a subsection of *ingrid* grid. If this subsection
    exceeds the boundaries of the grid, only the common region will be extracted.
.. include:: explain_-R.rst_

.. _-S:

**-S**\ *slopefile*
    Name of output grid file with scalar magnitudes of gradient vectors.
    Requires **-D** but makes **-G** optional.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

|SYN_OPT-f|
   Geographic grids (dimensions of longitude, latitude) will be converted to
   meters via a "Flat Earth" approximation using the current ellipsoid parameters.

.. include:: explain_-n.rst_

.. include:: explain_help.rst_

Grid Distance Units
-------------------

If the grid does not have meter as the horizontal unit, append **+u**\ *unit* to the input file name to convert from the
specified unit to meter.  If your grid is geographic, convert distances to meters by supplying |SYN_OPT-f| instead.

Hints
-----

If you don't know what **-N** options to use to make an intensity file
for :doc:`grdimage` or :doc:`grdview`, a good first try is **-Ne**\ 0.6.

Usually 255 shades are more than enough for visualization purposes. You
can save 75% disk space by appending =nb/a to the output filename
*outgrid*.

If you want to make several illuminated maps of subregions of a large
data set, and you need the illumination effects to be consistent across
all the maps, use the **-N** option and supply the same value of *sigma*
and *offset* to **grdgradient** for each map. A good guess is *offset* =
0 and *sigma* found by :doc:`grdinfo` **-L2** or **-L1** applied to an
unnormalized gradient grd.

If you simply need the *x*- or *y*-derivatives of the grid, use :doc:`grdmath`.

.. include:: explain_grd_inout_short.rst_

Tiles
-----

For very large datasets (or very large plots) you may need to break the job into multiple
tiles. It is then important that the normalization of the intensities are handled the
same way for each tile.  By default, *offset* and *sigma* are recalculated for each tile.
Hence, different tiles of the same large grid will compute different *offset* and *sigma* values.
Thus, the intensity for the same directional slope will be different across the final map.
This inconsistency can lead to visible changes in image appearance across tile seams.
The way to ensure compatible results is to specify the same *offset* and *sigma* via
the modifiers to **-N**.  However, if these need to be estimated from the large grid then
the **-Q** option can help: Run **grdgradient** on the full grid (or as large portion of
the grid that your computer can handle) and specify **-Qc** to create a statistics file
with the resulting *offset* and *sigma*.  Then, for each of your grid tile calculations, give
**+o** and/or **+s** without arguments to **-N** and specify **-Qr**.  This option will read
the values from the hidden statistics file and use them in the normalization.
If you use **-QR** for the final tile then the statistics file is removed after use.

Ambient
-------

The *ambient* light offset is used to darken or brighten all intensities.  This
modifier is typically used to darken an entire image by subtracting a constant from
all the intensities.  E.g., if you use **+a**\ -0.5 then you subtract 0.5 from all
intensities, making them more negative and hence darken the image.

Examples
--------

.. include:: explain_example.rst_

To make a file for illuminating a portion of the data in the remote file @earth_relief_05 using
exponentially  normalized gradients in the range [-0.6,0.6] imitating light sources in
the north and west directions::

    gmt grdgradient @earth_relief_05m -R0/20/0/20 -A0/270 -Ggradients.nc -Ne0.6 -V

To find the azimuth orientations of seafloor fabric in the file topo.nc:

   ::

    gmt grdgradient topo.nc -Dno -Gazimuths.nc -V

To determine the offset and sigma suitable for normalizing the intensities from topo.nc, do

   ::

    gmt grdgradient topo.nc -A30 -Nt0.6 -Qc -V

Without **-G**, only the hidden statistics file is created and no output grid is written.

To use the previously determined offset and sigma to normalize the intensities in tile_3.nc, do

   ::

    gmt grdgradient tile_3.nc -A30 -Nt0.6+o+s -Qr -V -Gtile_3_int.nc


References
----------

Horn, B.K.P., Hill-Shading and the Reflectance Map, Proceedings of the
IEEE, Vol. 69, No. 1, January 1981, pp. 14-47.
(http://people.csail.mit.edu/bkph/papers/Hill-Shading.pdf)

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`
:doc:`grdhisteq`,
:doc:`grdinfo`,
:doc:`grdmath`,
:doc:`grdimage`, :doc:`grdview`,
:doc:`grdvector`
