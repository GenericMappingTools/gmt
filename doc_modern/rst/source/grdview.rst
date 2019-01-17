.. index:: ! grdview

*******
grdview
*******

.. only:: not man

    Create 3-D perspective image or surface mesh from a grid

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdview** *reliefgrid* |-J|\ *parameters*
[ |SYN_OPT-B| ]
[ |-C|\ [*cpt*]]
[ |-G|\ *drapegrid* \| |-G|\ *grd_r* |-G|\ *grd_g* |-G|\ *grd_b* ]
[ |-I|\ [*intensgrid*\ \|\ *intensity*\ \|\ *modifiers*] ]
[ **-Jz**\ \|\ **Z**\ *parameters* ]
[ |-N|\ *level*\ [**+g**\ *fill*] ]
[ |-Q|\ *args*\ [**+m**] ]
[ |SYN_OPT-Rz| ]
[ |-S|\ *smooth* ]
[ |-T|\ [\ **+o**\ [*pen*]][**+s**] ]
[ |SYN_OPT-U| ]
[ |-W|\ **c|m|f**\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdview** reads a 2-D grid file and produces a 3-D perspective plot by
drawing a mesh, painting a colored/gray-shaded surface made up of
polygons, or by scanline conversion of these polygons to a raster image.
Options include draping a data set on top of a surface, plotting of
contours on top of the surface, and apply artificial illumination based
on intensities provided in a separate grid file. 

Required Arguments
------------------

*reliefgrid*
    2-D gridded data set to be imaged (the relief of the surface). (See
    GRID FILE FORMAT below.) 

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: explain_-J.rst_

.. include:: explain_-Jz.rst_

Optional Arguments
------------------

.. _-B:

.. include:: explain_-B.rst_

.. _-C:

**-C**\ [*cpt* \|\ *master*\ [**+i**\ *zinc*] \|\ *color1,color2*\ [,\ *color3*\ ,...]]
    The name of the CPT. Must be present if you want
    (1) mesh plot with contours (**-Qm**), or
    (2) shaded/colored perspective image (**-Qs** or
    **-Qi**). For **-Qs**: You can specify that you want to skip a
    z-slice by setting the red r/g/b component to -; to use a pattern give red =
    **P\|p**\ *pattern*\ [**+b**\ *color*\ ][**+f**\ *color*\ ][**+r**\ *dpi*\ ].
    Alternatively, supply the name of a GMT color master dynamic CPT [rainbow] to
    automatically determine a continuous CPT from
    the grid's z-range; you may round up/down the z-range by adding **+i**\ *zinc*..

.. _-G:

|-G|\ *drapegrid* \| |-G|\ *grd_r* |-G|\ *grd_g* |-G|\ *grd_b*
    Drape the image in *drapegrid* on top of the relief provided by
    *reliefgrid*. [Default determines colors from *reliefgrid*]. Note that **-Jz** and
    **-N** always refers to the *reliefgrid*. The *drapegrid* only
    provides the information pertaining to colors, which (if *drapegrid* is a grid) will be looked-up
    via the CPT (see **-C**). Instead, you may give three grid files
    via separate **-G** options in the specified order. These files must contain the red, green, and
    blue colors directly (in 0-255 range) and no CPT is needed. The
    *drapegrid* may be of a different resolution than the *reliefgrid*.
    Finally, *drapegrid* may be an image to be draped over the surface, in which
    case the **-C** option is not required.

.. _-I:

**-I**\ [*intensgrid*\ \|\ *intensity*\ \|\ *modifiers*]
    Gives the name of a grid file with intensities in the (-1,+1) range,
    or a constant intensity to apply everywhere; this simply affects the
    ambient light.  If just **+** is given then we derive an intensity
    grid from the input data grid *reliefgrid* via a call to :doc:`grdgradient`
    using the arguments **-A**\ -45 and **-Nt**\ 1 for that module. You can
    append **+a**\ *azimuth* and **+n**\ *args* to override those values.  If you want
    more specific intensities then run :doc:`grdgradient` separately first.
    [Default is no illumination].

.. _-N:

**-N**\ *level*\ [**+g**\ *fill*]
    Draws a plane at this z-level. If the optional *color* is provided
    via the **+g** modifier,
    the frontal facade between the plane and the data perimeter is
    colored. See **-Wf** for setting the pen used for the outline. 

.. _-Q:

**-Q**\ *args*\ [**+m**]
    Select one of following settings. For any of these choices, you may force
    a monochrome image by appending the modifier **+m**. Colors are then
    converted to shades of gray using the (monochrome television) YIQ transformation

    #. Specify **m** for mesh plot [Default], and optionally append *color* for a different mesh paint [white].
    #. Specify **mx** or **my** for waterfall plots (row or column profiles). Specify color as for plain **m**
    #. Specify **s** for surface plot, and optionally append **m** to have mesh lines drawn on top of surface.
    #. Specify **i** for image plot, and optionally append the effective dpi resolution for the rasterization [100].
    #. Specify **c**. Same as **-Qi** but will make nodes with z = NaN transparent, using the colormasking
       feature in PostScript Level 3 (the PS device must support PS Level 3). . 

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. |Add_-Rz| replace:: This option may be used to
    indicate the range used for the 3-D axes [Default is region given by
    the *reliefgrid*]. You may ask for a larger *w/e/s/n* region to
    have more room between the image and the axes. A smaller region than
    specified in the *reliefgrid* will result in a subset of the grid.
.. include:: explain_-Rz.rst_

.. _-S:

**-S**\ *smooth*
    Smooth the contours before plotting (see :doc:`grdcontour`) [Default is no smoothing].

.. _-t:

**-T**\ [\ **+o**\ [*pen*]][**+s**]
    Plot image without any interpolation. This involves converting each
    node-centered bin into a polygon which is then painted separately.
    Append **+s** to skip nodes with z = NaN. This option is useful for
    categorical data where interpolating between values is meaningless.
    Optionally, append **+o** to draw the tile outlines, and specify a
    custom pen if the default pen is not to your liking. As this option
    produces a flat surface it cannot be combined with **-JZ** or **-Jz**. 

.. _-U:

.. include:: explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ **c**\ \|\ **m**\ \|\ **f**\ *pen*

    **-Wc**
        Draw contour lines on top of surface or mesh (not image). Append pen
        attributes used for the contours. [Default: width = 0.75p, color =
        black, style = solid].
    **-Wm**
        Sets the pen attributes used for the mesh. [Default: width = 0.25p,
        color = black, style = solid]. You must also select **-Qm** or
        **-Qsm** for meshlines to be drawn.
    **-Wf**
        Sets the pen attributes used for the facade. [Default: width =
        0.25p, color = black, style = solid]. You must also select **-N**
        for the facade outline to be drawn. 

.. _-X:

.. include:: explain_-XY.rst_

.. include:: explain_-n.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-t.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout_short.rst_

.. include:: explain_grdresample.rst_

Examples
--------

To make a mesh plot from the file hawaii_grav.nc and drawing the
contours given in the CPT hawaii.cpt on a Lambert map at
1.5 cm/degree along the standard parallels 18 and 24, with vertical
scale 20 mgal/cm, and looking at the surface from SW at 30 degree
elevation, run

   ::

    gmt grdview hawaii_grav.nc -Jl18/24/1.5c -Chawaii.cpt
                -Jz0.05c -Qm -N-100 -p225/30 -Wc -pdf hawaii_grav_image

To create a illuminated color perspective plot of the gridded data set
image.nc, using the CPT color.cpt, with linear scaling at
10 cm/x-unit and tickmarks every 5 units, with intensities provided by
the file intens.nc, and looking from the SE, use

   ::

    gmt grdview image.nc -Jx10c -Ccolor.cpt -Qs -p135/30 -Iintens.nc -pdf image3D

To make the same plot using the rastering option with dpi = 50, use

   ::

    gmt grdview image.nc -Jx10c -Ccolor.cpt -Qi50 -p135/30 -Iintens.nc -pdf image3D

To create a color PostScript perspective plot of the gridded data set
magnetics.nc, using the CPT mag_intens.cpt, draped over
the relief given by the file topography.nc, with Mercator map width of 6
inch and tickmarks every 1 degree, with intensities provided by the file
topo_intens.nc, and looking from the SE, run

   ::

    gmt grdview topography.nc -JM6i -Gmagnetics.nc -Cmag_intens.cpt
                -Qs -p140/30 -Itopo_intens.nc -pdf draped3D

Remarks
-------

For the **-Qs** option: The PostScript language has no mechanism for smoothly varying
colors within a polygon, so colors can only vary from polygon to
polygon. To obtain smooth images this way you may resample the grid
file(s) using :doc:`grdsample` or use a finer grid size when running
gridding programs like :doc:`surface` or :doc:`nearneighbor`. Unfortunately,
this produces huge PostScript files. The alternative is to use the
**-Qi** option, which computes bilinear or bicubic continuous color
variations within polygons by using scanline conversion to image the polygons.

See Also
--------

:doc:`gmt`,
:doc:`gmtcolors`,
:doc:`grdcontour`,
:doc:`grdimage`,
:doc:`grdsample`,
:doc:`nearneighbor`,
:doc:`basemap`,
:doc:`contour`, :doc:`text`,
:doc:`surface`
