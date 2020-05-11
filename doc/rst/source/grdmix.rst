.. index:: ! grdmix
.. include:: module_core_purpose.rst_

******
grdmix
******

|grdmix_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdmix**
*raster1* [ *raster2* [ *raster3*]]
|-G|\ *outfile*
[ |-A|\ *weights* ]
[ |-C| ]
[ |-D| ]
[ |-I|\ *intens* ]
[ |-M| ]
[ |-N| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdmix** will perform various operations involving images and grids.
We either use a *weights* grid, image, or constant to add a new alpha
(transparency) layer to the image given as *raster1*, or we will blend
the two *raster1* and *raster2* (grids or images) using the *weights* for
*raster1* and the complementary *1 - weights* for *raster2* and save to
*outfile*. Alternatively, we will deconstruct an image into its component
grids or construct an image from its normalized component grids.
All *raster?*, *weights* and *intens* files must have the same 
dimensions. The optional *weights* and *intens* files may be replaced by
constant values instead.

Required Arguments
------------------

*raster?*
    If only one is given and **-C** is not set then *raster1* must be an image.
    If two are given then *raster1* and *raster2* must both be either
    images or grids.  If three are given then they must all be grids and
    **-C** must be set.

.. _-G:

**-G**\ *outfile*
    The name for the output raster.

Optional Arguments
------------------

.. _-A:

**-A**\ *weights*
    A constant weight (0-1), or a grid (0-1) or image (0-255) with weights.
    When two input rasters are given, the weights are applied to *raster1* and
    (*1-weights*) are applied to *raster2*, then summed.  For other operations
    the *weights* will be interpreted as the alpha (transparency) values.

.. _-C:

**-C**
    **C**\ onstruct an output image from one or three normalized input grids;
    these grids must all have values in the 0-1 range only.
    Optionally, use **-A** to include transparency and **-I** to add intensity
    to the colors before writing the image.

.. _-D:

**-D**
    **D**\ eonstruct a single image into one or three normalized output grids.
    An extra grid will be written if the image contains an alpha (transparency layer).
    All grids written will have values in the 0-1 range exclusively.
    The output names uses the name template given by **-G** which must contain the
    C-format code "%c".  This code is replaced by the codes R, G, B and A for color
    images and g, A for gray-scale images.

.. _-I:

**-I*\ *intens*
    A constant intensity or grid (-1/+1 range) to modify final output image colors.

.. _-M:

**-M**
    Force conversion to monochrome image using the (television) YIQ
    transformation.
.. _-N:

**-N**
    Normalize all input grids from 0-255 to 0-1 [All input grids already in 0-1 range].

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

To break the color image layers.png into separate, normalized red, green blue grids (and possibly an alpha grid),
we run::

    gmt grdmix layers.png -Glayer_%c.grd

To recombine the three normalized grids red.grd, green.grd, and blue.grd into a TIFF file, but
applying intensities from intens.grd and add transprency from transp.grd grids, try::

    gmt grdmix red.grd green.grd and blue.grd -Glayer.png -Atransp.grd -Iintens.grd


See Also
--------

:doc:`gmt`, :doc:`grdblend`,
:doc:`grdclip`,
:doc:`grdcut`,
:doc:`grdinfo`,
:doc:`grdsample`
