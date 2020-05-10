.. index:: ! grdmix
.. include:: module_core_purpose.rst_

******
grdmix
******

|grdmix_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdmix** *raster1* [ *raster2* ] [ |-A|\ *weights* ] |-G|\ *outfile*
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdmix** will either use the *weights* grid, image, or constant to add a
transparency layer to the image given as *raster1*, or it will blend
the two *raster1* and *raster2* (grids or images) using the *weights* for
*raster1* and the complementary *1 - weights* for *raster2* and save to
*outfile*. Files *raster1*, *weights* and *raster2* must have the same 
dimensions. The *weights* file may optionally be replaced by a constant weight.

Required Arguments
------------------

*raster1*
    Grid or image. If *raster2* is not given then *raster1* must be an image.

.. _-A:

**-A**\ *weights*
    A constant weight (0-1), or a grid (0-1) or image (0-255) with weights.
    When two input rasters are given, the weights are applied to *raster1* and
    (*1-weights*) are applied to *raster2*, then summed.

.. _-G:

**-G**\ *outfile*
    The name for the output raster.

Optional Arguments
------------------

*raster2*
    Grid or image to be blended with *raster1* using the weights given in **-A**.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout_short.rst_

Examples
--------

.. include:: explain_example.rst_

Suppose map1.png and map2.png are overlapping maps of different quantities, but we wish
to use the image visible.png to blend them into a single image.  We try::

    gmt grdmix map1.png map2.png -Avisible.png -Gnewmap.png -V

To insert the values from the grid weights.grd into the image gravity.tif as an alpha
(transparency) layer, and write out a transparent PNG image, try::

    gmt grdmix gravity.tif -Aweights.grd -Gmap.png

See Also
--------

:doc:`gmt`, :doc:`grdblend`,
:doc:`grdclip`,
:doc:`grdcut`,
:doc:`grdinfo`,
:doc:`grdsample`
