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
[ |-A|\ *alpha* ]
[ |-C| ]
[ |-D| ]
[ |-I|\ *intensity* ]
[ |-M| ]
[ |-N|\ [*factor*] ]
[ |-Q| ]
[ |SYN_OPT-R| ]
[ |SYN_OPT-V| ]
[ |-W|\ *weights* ]
[ |SYN_OPT-f| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdmix** will perform various operations involving images and grids.
We either use a *alpha* grid, image, or constant to add a new alpha
(transparency) layer to the image given as *raster1*, or we will blend
the two *raster1* and *raster2* (grids or images) using the *weights* for
*raster1* and the complementary *1 - weights* for *raster2* and save to
*outfile*. Alternatively, we will deconstruct an image into its component
(red, green, blue or gray) grid layers or we construct an image from its
normalized component grids.
All operations support adjusting the final color image via an *intensity*
grid, converting a color image to monochrome, or strip off the alpha layer.
All *raster?*, *alpha*, *intensity* and *weights* files must have the same 
dimensions. The optional *alpha*, *intensity* and *weights* files may be
replaced by constant values instead.

Required Arguments
------------------

*raster?*
    If only one is given and **-C** is not set then *raster1* must be an image.
    If two are given then *raster1* and *raster2* must both be either
    images or grids.  If three are given then they must all be grids and
    **-C** must be set.

.. _-G:

**-G**\ *outfile*
    The name for the output raster.  For images, use one of these extensions:
    tif (GeoTIFF), gif, png, jpg, bmp, or ppm.

Optional Arguments
------------------

.. _-A:

**-A**\ *alpha*
    Get a constant alpha (0-1), or a grid (0-1) or image (0-255) with alphas.
    The final image will have a transparency layer add based on these values.

.. _-C:

**-C**
    **C**\ onstruct an output image from one or three normalized input grids;
    these grids must all have values in the 0-1 range only (see **-N** if they don't).
    Optionally, use **-A** to add transparency and **-I** to add intensity
    to the colors before writing the image. For three layers the input order must
    be red grid first, then the green grid, and finally the blue grid.

.. _-D:

**-D**
    **D**\ econstruct a single image into one or three normalized output grids.
    An extra grid will be written if the image contains an alpha (transparency layer).
    All grids written will have values in the 0-1 range exclusively; however, you
    can use **-N** to undo this implicit normalization.
    The output names uses the name template given by **-G** which must contain the
    C-format string "%c".  This code is replaced by the codes R, G, B and A for color
    images and g, A for gray-scale images.

.. _-I:

**-I**\ *intensity*
    A constant intensity or grid (-1/+1 range) to modify final output image colors.

.. _-M:

**-M**
    Force conversion to monochrome image using the (television) YIQ
    transformation.

.. _-N:

**-N**\ [*factor*] 
    Normalize all input grids from 0-255 to 0-1 [All input grids are assumed to be in 0-1 range].
    To normalize by another factor than 255, append an optional *factor* value.

.. _-Q:

**-Q**
    Make the final image opaque by removing the alpha layer (if present).

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ *weights*
    A constant weight (0-1), or a grid (0-1) or image (0-255) with weights.
    When two input rasters are given, the weights are applied to *raster1* and
    (*1-weights*) are applied to *raster2*, then the products are summed.

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout_short.rst_

Examples
--------

.. include:: explain_example.rst_

To blend the night and day views of the Earth using a weight image computed for
a particular day/night terminus, try::

    gmt grdmix @BlueMarble_06m.tif @BlackMarble_06m.tif -W@weight.png -Gnewmap.png

Suppose map1.png and map2.png are overlapping maps of different quantities, but we wish
to use the image visible.png to blend them into a single image: Where visible.png has
values close to 255 we will see predominantly the map1.png contents while for values
closer to zero we will mostly see map2.png - values in between these extremes will
lead to a weighted average.  We try::

    gmt grdmix map1.png map2.png -Wvisible.png -Gnewmap.png -V

To insert the values from the grid transparency.grd into the image gravity.tif as an alpha
(transparency) layer, and write out a transparent PNG image, try::

    gmt grdmix gravity.tif -Atransparency.grd -Gmap.png

To break the color image layers.png into separate, normalized red, green, and blue grids (and possibly an alpha grid),
we run::

    gmt grdmix layers.png -D -Glayer_%c.grd

To recombine the three normalized grids red.grd, green.grd, and blue.grd into a TIFF file, but
applying intensities from intens.grd and add transparency from transp.grd grids, try::

    gmt grdmix red.grd green.grd and blue.grd -Glayer.tif -Atransp.grd -Iintens.grd


See Also
--------

:doc:`gmt`, :doc:`grdblend`,
:doc:`grdclip`,
:doc:`grdcut`,
:doc:`grdinfo`,
:doc:`grdsample`
